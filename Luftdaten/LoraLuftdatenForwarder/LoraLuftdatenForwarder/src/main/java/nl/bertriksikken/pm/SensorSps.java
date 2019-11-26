package nl.bertriksikken.pm;

import java.util.Locale;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * Data transfer object for SPS30 sensor data.
 * paulvha / November 2019
 */
public final class SensorSps {

    @JsonProperty("id")
    private String id;
    @JsonProperty("PM10")
    private double pm10;
    @JsonProperty("PM2.5")
    private double pm2_5;
    @JsonProperty("PM1")
    private double pm1;

    private SensorSps() {
        // jackson constructor
    }

    /**
     * Constructor.
     *
     * @param id    SPS30  ID
     * @param pm10  the PM10 value
     * @param pm2_5 the PM2.5 value
     * @param pm1   the PM1 value
     */
    public SensorSps(String id, double pm10, double pm2_5, double pm1) {
        this();
        this.id = id;
        this.pm10 = pm10;
        this.pm2_5 = pm2_5;
        this.pm1 = pm1;
    }

    public String getId() {
        return id;
    }

    public double getPm10() {
        return pm10;
    }

    public double getPm2_5() {
        return pm2_5;
    }

    public double getPm1() {
        return pm1;
    }
    @Override
    public String toString() {
        return String.format(Locale.US, "{id=%s,PM10=%.1f,PM2.5=%.1f,PM1=%.1f}", id, pm10, pm2_5,pm1);
    }

}
