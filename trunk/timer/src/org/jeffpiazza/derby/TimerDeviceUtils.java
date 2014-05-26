package org.jeffpiazza.derby;

import jssc.*;
import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;


public class TimerDeviceUtils {
    private static final Pattern finishPattern = Pattern.compile("([A-F]=(\\d\\.\\d+).?)( [A-F]=(\\d\\.\\d+).?)*$");
    private static final Pattern singleLanePattern = Pattern.compile("([A-F])=(\\d\\.\\d+)([^ ]?)");

    public static String[] parseCommonRaceResult(String line, int nlanes) {
        Matcher m = finishPattern.matcher(line);
        if (m.matches()) {
            String[] results = new String[nlanes];
            m = singleLanePattern.matcher(line);
            for (int i = 0; i < line.length() && m.find(i); i = m.end() + 1) {
                int index = m.group(1).charAt(0) - 'A';
                results[index] = m.group(2);
                // TODO: Place information
            }
                        
            return results;
        }
        return null;
    }
}
