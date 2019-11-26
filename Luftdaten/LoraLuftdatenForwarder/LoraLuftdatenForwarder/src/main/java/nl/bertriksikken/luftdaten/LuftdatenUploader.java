package nl.bertriksikken.luftdaten;

import java.io.IOException;
import java.time.Duration;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.databind.ObjectMapper;

import nl.bertriksikken.luftdaten.dto.LuftdatenMessage;
import okhttp3.OkHttpClient;
import retrofit2.Response;
import retrofit2.Retrofit;
import retrofit2.converter.jackson.JacksonConverterFactory;
import retrofit2.converter.scalars.ScalarsConverterFactory;

/**
 * Uploader for luftdaten.info
 * adjusted for SPS30 / paulvha / November 2019
 */
public final class LuftdatenUploader {

        private static final Logger LOG = LoggerFactory.getLogger(LuftdatenUploader.class);

        // the "PIN" we upload dust data to
        public static final String PIN_SPS = "1";
        public static final String PIN_BME = "11";

        private final ObjectMapper mapper = new ObjectMapper();
        private final ILuftdatenApi restClient;

        /**
         * Constructor.
         *
         * @param restClient the REST client
         */
        public LuftdatenUploader(ILuftdatenApi restClient) {
                this.restClient = restClient;
        }

        /**
         * Creates a new REST client.
         *
         * @param url the URL of the server, e.g. "https://api.luftdaten.info"
         * @param timeout the timeout
         * @return a new REST client.
         */
        public static ILuftdatenApi newRestClient(String url, Duration timeout) {
            LOG.info("Creating new REST client for '{}' with timeout {}", url, timeout);

            OkHttpClient client = new OkHttpClient().newBuilder().callTimeout(timeout)
                                .build();
                Retrofit retrofit = new Retrofit.Builder()
                                .baseUrl(url)
                                .addConverterFactory(ScalarsConverterFactory.create())
                                .addConverterFactory(JacksonConverterFactory.create())
                                .client(client)
                                .build();

                return retrofit.create(ILuftdatenApi.class);
        }

    public void uploadMeasurement(String sensorId, String pin, LuftdatenMessage luftdatenMessage, int DebugDataflow) {
        try {

            // only display if extended debug is enabled
            if (DebugDataflow == 1)  {
                LOG.info("Sending for {} to pin {}: '{}'", sensorId, pin, mapper.writeValueAsString(luftdatenMessage));
            }

            Response<String> response = restClient.pushSensorData(pin, sensorId, luftdatenMessage).execute();

            // only display if extended debug is enabled
            if (DebugDataflow == 1)  {
                if (response.isSuccessful()) {
                        LOG.info("Result success: {}", response.body());
                } else {
                        LOG.warn("Request failed: {}", response.message());
                }
            }
            else { // display warning anyway if not succesfull
                if ( ! response.isSuccessful()) LOG.warn("Request failed: {}", response.message());
            }

        } catch (IOException e) {
                LOG.warn("Caught exception '{}'", e.getMessage());
        }
    }

        public void start() {
                LOG.info("Starting Luftdaten.info uploader");
        }

        public void stop() {
                LOG.info("Stopping Luftdaten.info uploader");
        }

}
