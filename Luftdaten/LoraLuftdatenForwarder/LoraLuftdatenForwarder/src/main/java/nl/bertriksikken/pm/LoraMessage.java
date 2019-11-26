package nl.bertriksikken.pm;

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.text.ParseException;
import java.util.Optional;

/** adjusted for sps30
 * paulvha / november 2019
 */
public final class LoraMessage {

    private final double pm1;
    private final double pm10;
    private final double pm2_5;
    private final Optional<Double> temp;
    private final Optional<Double> humidity;
    private final Optional<Double> pressure;

    public LoraMessage(double pm10, double pm2_5,double pm1, Optional<Double> temp, Optional<Double> humidity, Optional<Double> pressure) {
        this.pm10 = pm10;
        this.pm2_5 = pm2_5;
        this.pm1 = pm1;
        this.temp = temp;
        this.humidity = humidity;
        this.pressure = pressure;
    }

    public double getPm1() {
        return pm1;
    }

    public double getPm10() {
        return pm10;
    }

    public double getPm2_5() {
        return pm2_5;
    }

    public Optional<Double> getTemp() {
        return temp;
    }

    public Optional<Double> getHumidity() {
        return humidity;
    }

    public Optional<Double> getPressure() {
        return pressure;
    }

    // extract the values from the lora payload
    // this NOT used as Payload is extracted in TTN
    public static LoraMessage decode(byte[] data) throws ParseException {
        ByteBuffer bb = ByteBuffer.wrap(data).order(ByteOrder.BIG_ENDIAN);
        try {
            int rawPm10 = bb.getShort() & 0xFFFF;
            double pm10 = rawPm10 / 10.0;
            int rawPm2_5 = bb.getShort() & 0xFFFF;
            double pm2_5 = rawPm2_5 / 10.0;
            int rawPm1 = bb.getShort() & 0xFFFF;
            double pm1 = rawPm1 / 10.0;
            int rawTemp = bb.getShort();
            Optional<Double> temp = getOptional(rawTemp, 10.0);
            int rawHumi = bb.getShort();
            Optional<Double> humi = getOptional(rawHumi, 10.0);
            int rawPres = bb.getShort();
            Optional<Double> press = getOptional(rawPres, 10.0);
            return new LoraMessage(pm10, pm2_5, pm1, temp, humi,press);
        } catch (BufferUnderflowException e) {
            throw new ParseException("underflow", bb.position());
        }
    }

    private static Optional<Double> getOptional(int rawValue, double scale) {
        return ((rawValue & 0xFFFF) == 0xFFFF) ? Optional.empty() : Optional.of(rawValue / scale);
    }

}
