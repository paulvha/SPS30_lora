package nl.bertriksikken.loraforwarder;

import java.time.Duration;

/**
 * Configuration interface for the application.
 */
public interface ILoraForwarderConfig {

    /**
     * @return the URL of the MQTT server
     */
    String getMqttUrl();

    String getMqttAppId();

    String getMqttAppKey();

    /**
     * @return the payload encoding, e.g. "rudzl"
     */
    String getEncoding();

    /**
     * @return the URL of the luftdaten.info API
     */
    String getLuftdatenUrl();

    /**
     * @return timeout (ms) for accessing the luftdaten.info API
     */
    Duration getLuftdatenTimeout();

    /**
     * @return the storage directory for the data files
     */
    String getStorageDir();

    /**
     * @return the extension to add to datafile
     */
    String getExtension();

    /**
     * @return the 1 or 0
     */
    int getDebug();
}
