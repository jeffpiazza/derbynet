# Please visit us at [https://derbynet.org](https://derbynet.org).

![icon](https://raw.githubusercontent.com/jeffpiazza/derbynet/master/website/img/derbynet-300.png)

# Developing locally

To quickly get started on local development, the existing Docker image can be
used to provide the web server and PHP engine, even if you don't have these
installed natively on your machine.

1. Install [Apache Ant](https://ant.apache.org/). 
   1. You can [install WSL](https://learn.microsoft.com/en-us/windows/wsl/install) and run:

      ```bash
      sudo apt-get update
      sudo apt-get install ant
      ```

2. Execute `ant generated` from the root of the cloned repository.  (This build
target includes a step to generate PDF files from their ODF source files.  This
step will be silently skipped if the LibreOffice/OpenOffice `soffice`
application is not available.)

3. If desired, do one or both of the following.  (If you do neither, you won't
be able to connect to a hardware timer.)

   1. Execute `ant timer-in-brower` to build the in-browser timer interface.
   2. Execute `ant timer-jar` to build the derby-timer.jar timer interface.

4. Instantiate the docker container, but use your local sources rather than
those deployed in the container.  _**PATH_TO_YOUR_DATA**_ is a local directory
where you'd like databases, photos, and other data files to be stored.
_**PATH_TO_YOUR_REPOSITORY**_ is the path to your local cloned repository.

   ```powershell
   docker run --detach -p 80:80 -p 443:443 \
     --volume [** PATH TO YOUR DATA **]\lib\:/var/lib/derbynet \
     --mount type=bind,src=[** PATH TO YOUR REPOSITORY **]\website\,target=/var/www/html,readonly \
     jeffpiazza/derbynet_server   
   ```
