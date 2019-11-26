package nl.bertriksikken.pm;

import java.util.Locale;
import java.util.Optional;

/**
 * Representation of a message received from the MQTT stream.
 * adjusted for SPS30  paulvha/november 2019
 */
public final class SensorMessage {

    private SensorSps sps;

    private SensorBme bme;

    private SensorMessage() {
        // Jackson constructor
    }

    /**
     * Constructor.
     */
    public SensorMessage(SensorSps sps) {
        this();
        this.sps = sps;
    }

    public SensorSps getSps() {
        return sps;
    }

    public void setBme(SensorBme bme) {
        this.bme = bme;
    }

    public Optional<SensorBme> getBme() {
        return Optional.ofNullable(bme);
    }

    @Override
    public String toString() {
        return String.format(Locale.US, "{sps=%s,bme=%s}", sps, bme);
    }

}
