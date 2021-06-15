'use strict';


async function connect(params) {
  // params: baud, data, stop, parity

  // - Wait for the port to open.
  await port.open({ baudrate: params.baud,
                    dataBits: params.data,
                    stopBits: params.stop,  // 1 or 2
                    parity: params.parity /* none, even, odd */ });

  let decoder = new TextDecoderStream();               // TODO Global decoder?
  inputDone = port.readable.pipeTo(decoder.writable);  // TODO Global inputDone?
  inputStream = decoder.readable
    .pipeThrough(new TransformStream(new LineBreakTransformer()))
    .pipeThrough(new TransformStream(new JSONTransformer()));
  return inputStream;
}


async function identify(profiles) {
  let port = await navigator.serial.requestPort();

  while (true) {
    for (prof of profiles) {
      if (prof.hasOwnProperty('prober')) {
        if (await identify_one(prof)) {
          return true;
        }
      }
    }
  }
}


async function identify_one(profile) {
  let inputStream = await connect(profile.params);

  const encoder = new TextEncoderStream();
  let outputDone = encoder.readable.pipeTo(port.writable);  // TODO Global
  let outputStream = encoder.writable;  // TODO
  // TODO pre_probe
  writeToStream(profile.prober.probe, outputStream);
  // TODO Read for a second or so
  //  inputStream.cancel(), but that's not allowed while a reader is outstanding
  //  reader.cancel() is apparently the solution?
  //
  // inputStream.pipeTo( writableStream ), but if the writable stream (or its
  // "sink") throws an error, the readable stream cancels.
  //
  // The java implementation works based on receiving interrupts when there's
  // data to read.  The streams model seems to want to create an async read
  // loop, and provide for canceling the reader when closing.  The alternative
  // of piping to a writable stream seems to offer less control.
  //
  // Timed reads from the port wrapper can be accomplished with the "usual"
  // setTimeout to reject the promise (or resolve to null).

  let reader = inputStream.getReader();
}


function writeToStream(cmd, outStream) {
  const writer = outStream.getWriter();
  writer.write(cmd);
  writer.releaseLock();
}
