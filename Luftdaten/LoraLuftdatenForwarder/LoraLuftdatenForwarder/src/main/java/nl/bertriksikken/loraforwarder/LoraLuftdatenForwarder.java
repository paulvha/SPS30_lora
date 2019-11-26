package nl.bertriksikken.loraforwarder;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.time.*;
import java.time.format.DateTimeFormatter;
import java.nio.charset.StandardCharsets;
import java.io.IOException;
import java.time.Instant;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.eclipse.paho.client.mqttv3.MqttException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.databind.ObjectMapper;

import nl.bertriksikken.loraforwarder.rudzl.dto.RudzlMessage;
import nl.bertriksikken.luftdaten.ILuftdatenApi;
import nl.bertriksikken.luftdaten.LuftdatenUploader;
import nl.bertriksikken.luftdaten.dto.LuftdatenItem;
import nl.bertriksikken.luftdaten.dto.LuftdatenMessage;
import nl.bertriksikken.pm.SensorBme;
import nl.bertriksikken.pm.SensorMessage;
import nl.bertriksikken.pm.SensorSps;
import nl.bertriksikken.ttn.MqttListener;
import nl.bertriksikken.ttn.dto.TtnUplinkMessage;

/**
 * updated for SPS30 / paulvha / November 2019
 */
public final class LoraLuftdatenForwarder {

    private static final Logger LOG = LoggerFactory.getLogger(LoraLuftdatenForwarder.class);
    private static final String CONFIG_FILE = "loraluftdatenforwarder.properties";
    private static final String SOFTWARE_VERSION = "SPS30_November_2019-1.0.1";
    private static String StorageDir;       // hold the place for the local data files
    private static String FileDateExt;      // hold the date extension for the datafile.
    private static int DebugDataflow;

    private final MqttListener mqttListener;
    private final LuftdatenUploader uploader;
    private final ExecutorService executor;
    private final EPayloadEncoding encoding;

    public static void main(String[] args) throws IOException, MqttException {
        ILoraForwarderConfig config = readConfig(new File(CONFIG_FILE));
        LoraLuftdatenForwarder app = new LoraLuftdatenForwarder(config);
        app.start();
        Runtime.getRuntime().addShutdownHook(new Thread(app::stop));
    }

    // called from main (above)
    private LoraLuftdatenForwarder(ILoraForwarderConfig config) {
        ILuftdatenApi restClient = LuftdatenUploader.newRestClient(config.getLuftdatenUrl(),
                config.getLuftdatenTimeout());

        // Folder for local data file
        StorageDir = config.getStorageDir();
        if(StorageDir == null || StorageDir.isEmpty()){
            LOG.warn("No Storage directory defined. No data will be saved locally");
        }
        else {
            LOG.info("Data files will be saved to {}", StorageDir);
        }

        // enable dataflow message
        DebugDataflow = config.getDebug();
        if (DebugDataflow == 0) {
            LOG.info("Dataflow debug information disabled");
        }

        // get extension for data file
        FileDateExt = config.getExtension();

        uploader = new LuftdatenUploader(restClient);
        executor = Executors.newSingleThreadExecutor();
        encoding = EPayloadEncoding.fromId(config.getEncoding());
        // start listener and return with message from TTN.  Call the messageReceived below to handle.
        mqttListener = new MqttListener(this::messageReceived, config.getMqttUrl(), config.getMqttAppId(),
                config.getMqttAppKey());

        LOG.info("Luftdaten forwarder version {}", SOFTWARE_VERSION);
        LOG.info("Created new Luftdaten forwarder for encoding {}", encoding);
    }

    // package-private to allow testing
    void messageReceived(Instant instant, String topic, String message) {
        if (DebugDataflow == 1) LOG.info("Received: '{}'", message);

        // decode JSON
        ObjectMapper mapper = new ObjectMapper();
        TtnUplinkMessage uplink;
        try {
            uplink = mapper.readValue(message, TtnUplinkMessage.class);
        } catch (IOException e) {
            LOG.warn("Could not parse JSON: '{}'", message);
            return;
        }

        // set the luftdaten devicename to TTN-hardware serial HARDCODED
        String sensorId = String.format(Locale.ROOT, "TTN-%s", uplink.getHardwareSerial());

        SensorMessage sensorMessage = decodeTtnMessage(instant, sensorId, uplink);

        // schedule upload & save
        if (sensorMessage != null) {
            executor.execute(() -> handleMessageTask(sensorId, sensorMessage));
            executor.execute(() -> saveToFile(sensorId, sensorMessage));
        }
    }

    // extract the data from the received TTN message
    private SensorMessage decodeTtnMessage(Instant instant, String sensorId, TtnUplinkMessage uplinkMessage) {
        switch (encoding) {
        case RUDZL:
            RudzlMessage message = new RudzlMessage(uplinkMessage.getPayloadFields());
            SensorSps sps = new SensorSps(sensorId, message.getPM10(), message.getPM2_5(), message.getPM1());
            SensorMessage sensorMessage = new SensorMessage(sps);
            SensorBme bme = new SensorBme(message.getT(), message.getRH(), message.getP());
            sensorMessage.setBme(bme);
            return sensorMessage;
        default:
            return null;
        }
    }

    private void handleMessageTask(String sensorId, SensorMessage sensorMessage) {
        // forward to luftdaten, in an exception safe manner (order of data seems to be important)
        try {
            LuftdatenMessage spsMessage = new LuftdatenMessage(SOFTWARE_VERSION);
            spsMessage.addItem(new LuftdatenItem("P0", sensorMessage.getSps().getPm1()));
            spsMessage.addItem(new LuftdatenItem("P1", sensorMessage.getSps().getPm10()));
            spsMessage.addItem(new LuftdatenItem("P2", sensorMessage.getSps().getPm2_5()));

            uploader.uploadMeasurement(sensorId, LuftdatenUploader.PIN_SPS, spsMessage, DebugDataflow);

            if (sensorMessage.getBme().isPresent()) {
                SensorBme bme = sensorMessage.getBme().get();
                LuftdatenMessage bmeMessage = new LuftdatenMessage(SOFTWARE_VERSION);
                bmeMessage.addItem(new LuftdatenItem("temperature", bme.getTemp()));
                bmeMessage.addItem(new LuftdatenItem("humidity", bme.getRh()));
                bmeMessage.addItem(new LuftdatenItem("pressure", 100.0 * bme.getPressure()));
                uploader.uploadMeasurement(sensorId, LuftdatenUploader.PIN_BME, bmeMessage, DebugDataflow);
            }
        } catch (Exception e) {
            LOG.trace("Caught exception", e);
            LOG.warn("Caught exception: {}", e.getMessage());
        }
    }

    /**
     * Save data to local files.
     *
     * @param sensor_id : hardware ID  (name on Luftdaten)
     * @param sensorMessage : filled Sensormessage
     */
    public final void saveToFile(String sensorId,  SensorMessage sensorMessage) {

        // if NO directory provided, no save will be done
        if(StorageDir == null || StorageDir.isEmpty())return;

        // create date information
        ZoneId zid = ZoneId.of("Europe/Paris");

        // create an LocalDateTime object using now(zoneId)
        LocalDateTime lt = LocalDateTime.now(zid);

        // create unique extension for file as defined in configuration file
        DateTimeFormatter file_format = DateTimeFormatter.ofPattern(FileDateExt);
        String file_ext = lt.format(file_format);

        // create timestamp for data
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy:MM:dd:hh:mm");
        String formattedDateTime = lt.format(formatter);

        // check whether file exists already
        File tempFile = new File(StorageDir + sensorId  + "-" +  file_ext);
        boolean exists = tempFile.exists();

        //System.out.println("time is: " + formattedDateTime);

        // open for append (boolean = true)
        try (FileOutputStream fos = new FileOutputStream((StorageDir + sensorId  + "-" +  file_ext), true)) {

            try (Writer writer = new OutputStreamWriter(fos, StandardCharsets.US_ASCII)) {

                // if new file write header first
                if (! exists) {
                    writer.append("yyyy:MM:dd:hh:mm,P0,P2,P1,Temp,Hum,Pressure\n");
                }

                // add time stamp
                writer.append(formattedDateTime);

                // add SDS30 data
                writer.append("," + sensorMessage.getSps().getPm1());
                writer.append("," + sensorMessage.getSps().getPm2_5());
                writer.append("," + sensorMessage.getSps().getPm10());

                // add BME data (if available)
                if (sensorMessage.getBme().isPresent()) {
                    SensorBme bme = sensorMessage.getBme().get();
                    writer.append("," + bme.getTemp());
                    writer.append("," + bme.getRh());
                    writer.append("," + bme.getPressure());
                    writer.append("\n");
                }
                else {
                    writer.append(",0,0,0\n");
                }

             } catch (Exception e) {
                LOG.trace("Caught exception in data creation", e);
                LOG.warn("Caught exception: {}", e.getMessage());
            }

            fos.close();
        } catch (Exception e) {
                LOG.trace("Caught exception in file handling", e);
                LOG.warn("Caught exception: {}", e.getMessage());
        }
    }

    /**
     * Starts the application.
     *
     * @throws MqttException in case of a problem starting MQTT client
     */
    private void start() throws MqttException {
        LOG.info("Starting LoraLuftdatenForwarder application");

        // start sub-modules
        uploader.start();
        mqttListener.start();

        LOG.info("Started LoraLuftdatenForwarder application");
    }

    /**
     * Stops the application.
     *
     * @throws MqttException
     */
    private void stop() {
        LOG.info("Stopping LoraLuftdatenForwarder application");

        mqttListener.stop();
        executor.shutdown();
        uploader.stop();

        LOG.info("Stopped LoraLuftdatenForwarder application");
    }

    // try loading configuration file
    // if it does not exist a new one will be created that can be edited

    private static ILoraForwarderConfig readConfig(File file) throws IOException {
        final LoraForwarderConfig config = new LoraForwarderConfig();
        try (FileInputStream fis = new FileInputStream(file)) {
            config.load(fis);
        } catch (IOException e) {
            LOG.warn("Failed to load config {}, writing defaults", file.getAbsoluteFile());
            try (FileOutputStream fos = new FileOutputStream(file)) {
                config.save(fos);   // create new file if it did not exist
            }
        }
        return config;
    }
}
