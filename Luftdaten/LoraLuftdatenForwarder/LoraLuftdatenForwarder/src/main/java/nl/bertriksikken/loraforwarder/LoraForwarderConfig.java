package nl.bertriksikken.loraforwarder;

import java.time.Duration;

/**
 * Configuration class. // updated for SPS30  paulvha / November 2019
 */
public final class LoraForwarderConfig extends BaseConfig implements ILoraForwarderConfig {

    private enum EConfigItem {
        MQTT_URL("mqtt.url", "tcp://eu.thethings.network", "URL of the MQTT server"),
        MQTT_APP_ID("mqtt.appid", "paulvha_app_a", "TTN application id"),
        MQTT_APP_KEY("mqtt.appkey", "ttn-account-v2.VEpepcjuuR8lATZ666sCkdRPRKFt7aLxMhv0qOlhFHk",
                "TTN application access key"),

        ENCODING("encoding", "rudzl", "The payload encoding"),

        LUFTDATEN_URL("luftdaten.url", "https://api.luftdaten.info", "luftdaten server URL (empty to disable)"),
        LUFTDATEN_TIMEOUT_MS("luftdaten.timeout", "10000", "luftdaten API timeout (milliseconds)"),

        STORAGE_DIR("storage.dir","/tmp/","storage directory for local data files (empty to disable)"),
        FILE_EXT("file.ext","YYMMdd","Format date extension for local data file"),

        ENABLE_DBG ("enable.dbg", "0", "1 will enable data flow messages, 0 will show program log only");


        private final String key, value, comment;

        private EConfigItem(String key, String defValue, String comment) {
            this.key = key;
            this.value = defValue;
            this.comment = comment;
        }
    }

    /**
     * Constructor.
     */
    public LoraForwarderConfig() {
        for (EConfigItem e : EConfigItem.values()) {
            add(e.key, e.value, e.comment);
        }
    }

    @Override
    public String getStorageDir() {
        return get(EConfigItem.STORAGE_DIR.key);
    }

    @Override
    public int getDebug() {
        //return get(EConfigItem.STORAGE_DIR.key);
        return (Integer.parseInt(get(EConfigItem.ENABLE_DBG.key)));
    }

    @Override
    public String getExtension() {
        return get(EConfigItem.FILE_EXT.key);
    }

    @Override
    public String getMqttUrl() {
        return get(EConfigItem.MQTT_URL.key);
    }

    @Override
    public String getMqttAppId() {
        return get(EConfigItem.MQTT_APP_ID.key);
    }

    @Override
    public String getMqttAppKey() {
        return get(EConfigItem.MQTT_APP_KEY.key);
    }

    @Override
    public String getLuftdatenUrl() {
        return get(EConfigItem.LUFTDATEN_URL.key).trim();
    }

    @Override
    public String getEncoding() {
        return get(EConfigItem.ENCODING.key).trim();
    }

    @Override
    public Duration getLuftdatenTimeout() {
        return Duration.ofMillis(Integer.parseInt(get(EConfigItem.LUFTDATEN_TIMEOUT_MS.key)));
    }

}
