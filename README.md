# Please visit us at [https://derbynet.org](https://derbynet.org).

![icon](https://raw.githubusercontent.com/jeffpiazza/derbynet/master/website/img/derbynet-300.png)

# Developing locally

To quickly get started on local development, the existing Docker image can be used.

1. Install [Apache Ant](https://ant.apache.org/). 
   1. You can [install WSL](https://learn.microsoft.com/en-us/windows/wsl/install) and run:

      ```bash
      sudo apt-get update
      sudo apt-get install ant
      ```

2. Execute `ant generated` from the root of the cloned repository
3. Instantiate the container:

   ```powershell
   docker run -d -p 80:80 -p443:443 -v [** PATH TO YOUR REPOSITORY **]\lib\:/var/lib/derbynet --mount type=bind,src=[** PATH TO YOUR REPOSITORY **]\website\,target=/var/www/html,readonly jeffpiazza/derbynet_server   
   ```
