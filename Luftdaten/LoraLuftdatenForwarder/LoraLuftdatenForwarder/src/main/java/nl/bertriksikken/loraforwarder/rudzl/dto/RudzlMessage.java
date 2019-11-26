package nl.bertriksikken.loraforwarder.rudzl.dto;

import java.util.HashMap;
import java.util.Map;

/** adjusted for SPS30 paulvha / November 2019
 */
public final class RudzlMessage {

    private final Map<String, Object> fields;

    public RudzlMessage(Map<String, Object> fields) {
        this.fields = new HashMap<>(fields);
    }

    public double getPM10() {
        Number number = (Number) fields.getOrDefault("PM10_Avg", Double.NaN);
        return number.doubleValue();
    }

    public double getPM2_5() {
        Number number = (Number) fields.getOrDefault("PM25_Avg", Double.NaN);
        return number.doubleValue();
    }

    public double getPM1() {
        Number number = (Number) fields.getOrDefault("PM1_Avg", Double.NaN);
        return number.doubleValue();
    }

    public double getT() {
        Number number = (Number) fields.getOrDefault("T", Double.NaN);
        return number.doubleValue();
    }

    public double getRH() {
        Number number = (Number) fields.getOrDefault("RH", Double.NaN);
        return number.doubleValue();
    }

    public double getP() {
        Number number = (Number) fields.getOrDefault("P", Double.NaN);
        return number.doubleValue();
    }

    public int getSpsId() {
        Number number = (Number) fields.getOrDefault("spsid", 0);
        return number.intValue() & 0xFFFF;
    }

}
