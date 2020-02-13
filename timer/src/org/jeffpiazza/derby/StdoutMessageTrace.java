package org.jeffpiazza.derby;

import java.io.*;
import javax.xml.transform.*;
import javax.xml.transform.stream.*;
import org.w3c.dom.Element;

public class StdoutMessageTrace {
  public static void httpMessage(Message m, String params) {
    System.out.println("===================== sending " + params);
  }

  public static void httpResponse(Message m, Element response) {
    System.out.println(prettyFormat(XmlSerializer.serialized(response)));
    System.out.println("=====================");
  }

  public static void trace(String s) {
    System.out.println(s);
    LogWriter.trace(s);
  }

  public static String prettyFormat(String input) {
    return prettyFormat(input, 2);
  }

  private static String prettyFormat(String input, int indent) {
    try {
      Source xmlInput = new StreamSource(new StringReader(input));
      StringWriter stringWriter = new StringWriter();
      StreamResult xmlOutput = new StreamResult(stringWriter);
      TransformerFactory transformerFactory = TransformerFactory.newInstance();
      transformerFactory.setAttribute("indent-number", indent);
      Transformer transformer = transformerFactory.newTransformer();
      transformer.setOutputProperty(OutputKeys.INDENT, "yes");
      transformer.transform(xmlInput, xmlOutput);
      return xmlOutput.getWriter().toString();
    } catch (Exception e) {
      throw new RuntimeException(e); // simple exception handling, please review it
    }
  }
}
