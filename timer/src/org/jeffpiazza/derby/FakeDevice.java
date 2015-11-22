package org.jeffpiazza.derby;

import jssc.SerialPortException;

import java.util.Random;

// For testing, add FakeDevice.class to
public class FakeDevice extends TimerDeviceBase implements TimerDevice {
  private Random random;

  public FakeDevice(SerialPortWrapper portWrapper) {
    super(portWrapper);
    this.random = new Random();
  }

  @Override
  public boolean probe() throws SerialPortException {
    // 50% chance of "discovering" our fake device on a given port
    return random.nextFloat() < 0.50;
  }

  @Override
  public int getNumberOfLanes() throws SerialPortException {
    return 5;
  }

  @Override
  public void prepareHeat(int laneMask) throws SerialPortException {
    System.out.println("FakeDevice.prepareHeat called");
  }

  @Override
  public void abortHeat() throws SerialPortException {
    System.out.println("FakeDevice.abortHeat called");
  }

  private long pollCount = 0;
  @Override
  public void poll() throws SerialPortException {
    ++pollCount;
    if ((pollCount % 1000) == 0 || (pollCount < 1000 && (pollCount % 100) == 0) || pollCount < 10) {
      System.out.println("FakeDevice.poll() called " + pollCount + " time(s).");
    }
  }
}
