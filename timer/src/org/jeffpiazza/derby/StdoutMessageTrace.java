package org.jeffpiazza.derby;

import org.w3c.dom.Element;

import java.io.*;
import java.util.ArrayList;
import java.util.regex.*;

import javax.xml.transform.*;
import javax.xml.transform.stream.*;

public class StdoutMessageTrace implements HttpTask.MessageTracer {
    public boolean traceResponses = false;

    public void onMessageSend(Message m, String params) {
        System.out.println("===================== sending " + params);
    }
    public void onMessageResponse(Message m, Element response) {
        if (traceResponses) {
            System.out.println(prettyFormat(XmlSerializer.serialized(response)));
            System.out.println("=====================");
        }
    }

  public void traceInternal(String s) {
    System.out.println(s);
  }

    public static String prettyFormat(String input, int indent) {
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

    public static String prettyFormat(String input) {
        return prettyFormat(input, 2);
    }
}
