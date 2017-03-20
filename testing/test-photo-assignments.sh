#! /bin/bash

# This only works when run on the server machine, not a separate client

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

curl_post action.php "action=settings.write&n-lanes=4" | check_success

curl_post action.php "action=photo.assign&repo=car&racer=1&photo=Car-1234.jpg" | check_failure
# For demo purposes, we want at least one car photo assigned early in the process
curl_post action.php "action=photo.assign&repo=car&racer=1&photo=Car-1637.jpg" | check_success

curl_post action.php "action=photo.assign&racer=1&photo=head-A.jpg" | check_success
curl_post action.php "action=photo.assign&racer=2&photo=head-B.jpg" | check_success
curl_post action.php "action=photo.assign&racer=3&photo=Cub-8759.jpg" | check_success
curl_post action.php "action=photo.assign&racer=4&photo=Cub-2825.jpg" | check_success
curl_post action.php "action=photo.assign&racer=5&photo=Cub-6429.jpg" | check_success
curl_post action.php "action=photo.assign&racer=6&photo=head-D.jpg" | check_success
curl_post action.php "action=photo.assign&racer=7&photo=Cub-1142.jpg" | check_success
curl_post action.php "action=photo.assign&racer=8&photo=Cub-32106.jpg" | check_success
curl_post action.php "action=photo.assign&racer=9&photo=Cub-7145.jpg" | check_success
curl_post action.php "action=photo.assign&racer=10&photo=Cub-1612.jpg" | check_success
curl_post action.php "action=photo.assign&racer=11&photo=head-C.jpg" | check_success
curl_post action.php "action=photo.assign&racer=12&photo=Cub-0117.jpg" | check_success
curl_post action.php "action=photo.assign&racer=13&photo=Cub-2878.jpg" | check_success
curl_post action.php "action=photo.assign&racer=14&photo=Cub-0707.jpg" | check_success
curl_post action.php "action=photo.assign&racer=15&photo=Cub-7530.jpg" | check_success
curl_post action.php "action=photo.assign&racer=16&photo=head-E.jpg" | check_success
curl_post action.php "action=photo.assign&racer=17&photo=Cub-0033.jpg" | check_success
curl_post action.php "action=photo.assign&racer=18&photo=Cub-1809.jpg" | check_success
curl_post action.php "action=photo.assign&racer=19&photo=Cub-9652.jpg" | check_success
curl_post action.php "action=photo.assign&racer=20&photo=Cub-5312.jpg" | check_success
curl_post action.php "action=photo.assign&racer=21&photo=head-F.jpg" | check_success
curl_post action.php "action=photo.assign&racer=22&photo=Cub-2995.jpg" | check_success
curl_post action.php "action=photo.assign&racer=23&photo=Cub-7962.jpg" | check_success
curl_post action.php "action=photo.assign&racer=24&photo=Cub-8200.jpg" | check_success
curl_post action.php "action=photo.assign&racer=25&photo=Cub-7232.jpg" | check_success
curl_post action.php "action=photo.assign&racer=26&photo=head-G.jpg" | check_success
curl_post action.php "action=photo.assign&racer=27&photo=Cub-0087.jpg" | check_success
curl_post action.php "action=photo.assign&racer=28&photo=Cub-2961.jpg" | check_success
curl_post action.php "action=photo.assign&racer=29&photo=Cub-6155.jpg" | check_success
curl_post action.php "action=photo.assign&racer=30&photo=Cub-7348.jpg" | check_success
# See test-photo-next.sh:
# curl_post action.php "action=photo.assign&racer=31&photo=Cub-9634.jpg" | check_success
curl_post action.php "action=photo.assign&racer=32&photo=Cub-1478.jpg" | check_success
curl_post action.php "action=photo.assign&racer=33&photo=Cub-0764.jpg" | check_success
# curl_post action.php "action=photo.assign&racer=34&photo=Cub-5517.jpg" | check_success
curl_post action.php "action=photo.assign&racer=35&photo=Cub-8464.jpg" | check_success
curl_post action.php "action=photo.assign&racer=36&photo=head-H.jpg" | check_success

curl_post action.php "action=photo.assign&repo=head&racer=37&photo=Cub-0330.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=38&photo=Cub-6494.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=39&photo=Cub-3126.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=40&photo=Cub-4356.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=41&photo=head-I.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=42&photo=Cub-6712.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=43&photo=Cub-6040.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=44&photo=Cub-4268.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=45&photo=Cub-7559.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=46&photo=head-J.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=47&photo=Cub-1280.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=48&photo=Cub-2418.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=49&photo=Cub-3714.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=50&photo=Cub-4622.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=51&photo=head-K.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=52&photo=Cub-9408.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=53&photo=Cub-0315.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=54&photo=Cub-4708.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=55&photo=Cub-0543.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=56&photo=head-L.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=57&photo=Cub-1376.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=58&photo=Cub-4352.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=59&photo=Cub-1451.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=60&photo=Cub-6386.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=61&photo=head-M.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=62&photo=Cub-3389.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=63&photo=Cub-1547.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=64&photo=Cub-0076.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=65&photo=Cub-8724.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=66&photo=head-N.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=67&photo=Cub-8590.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=68&photo=Cub-7883.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=69&photo=Cub-0533.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=70&photo=Cub-8188.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=71&photo=head-O.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=72&photo=Cub-4411.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=73&photo=Cub-3262.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=74&photo=Cub-4905.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=75&photo=Cub-9555.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=76&photo=head-P.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=77&photo=Cub-0435.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=78&photo=Cub-4863.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=79&photo=Cub-0324.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=80&photo=Cub-8330.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=81&photo=head-Q.jpg" | check_success
curl_post action.php "action=photo.assign&repo=head&racer=82&photo=Cub-6507.jpg" | check_success
# curl_post action.php "action=photo.assign&repo=head&racer=83&photo=Cub-7261.jpg" | check_success


curl_post action.php "action=photo.assign&repo=car&racer=1&photo=Car-1637.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=2&photo=Car-1638.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=3&photo=Car-1639.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=4&photo=Car-1640.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=5&photo=Car-1641.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=6&photo=Car-1642.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=7&photo=Car-1643.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=8&photo=Car-1644.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=9&photo=Car-1645.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=10&photo=Car-1646.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=11&photo=Car-1647.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=12&photo=Car-1648.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=13&photo=Car-1649.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=14&photo=Car-1650.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=15&photo=Car-1651.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=16&photo=Car-1652.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=17&photo=Car-1688.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=18&photo=Car-1845.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=19&photo=Car-1848.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=20&photo=Car-1849.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=21&photo=Car-1850.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=22&photo=Car-1851.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=23&photo=Car-1852.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=24&photo=Car-1853.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=25&photo=Car-1854.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=26&photo=Car-1855.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=27&photo=Car-1856.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=28&photo=Car-1857.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=29&photo=Car-1858.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=30&photo=Car-1859.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=31&photo=Car-1860.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=32&photo=Car-1861.jpg" | check_success
# See test-photo-next.sh
# curl_post action.php "action=photo.assign&repo=car&racer=33&photo=Car-1862.jpg" | check_success
# curl_post action.php "action=photo.assign&repo=car&racer=34&photo=Car-1863.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=35&photo=Car-1864.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=36&photo=Car-1865.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=37&photo=Car-1866.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=38&photo=Car-1867.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=39&photo=Car-1868.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=40&photo=Car-1869.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=41&photo=Car-1870.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=42&photo=Car-1871.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=43&photo=Car-1872.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=44&photo=Car-1873.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=45&photo=Car-1874.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=46&photo=Car-1875.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=47&photo=Car-1876.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=48&photo=Car-1877.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=49&photo=Car-1878.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=50&photo=Car-1879.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=51&photo=Car-1880.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=52&photo=Car-1881.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=53&photo=Car-1882.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=54&photo=Car-1883.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=55&photo=Car-5611.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=56&photo=Car-5612.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=57&photo=Car-5613.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=58&photo=Car-5614.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=59&photo=Car-5615.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=60&photo=Car-5616.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=61&photo=Car-5618.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=62&photo=Car-5619.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=63&photo=Car-5620.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=64&photo=Car-5621.jpg" | check_success
# These photo assignments should have already occurred in photo-setup.sh
# curl_post action.php "action=photo.assign&repo=car&racer=65&photo=Car-5622.jpg" | check_success
# curl_post action.php "action=photo.assign&repo=car&racer=66&photo=Car-5623.jpg" | check_success
# curl_post action.php "action=photo.assign&repo=car&racer=67&photo=Car-5624.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=68&photo=Car-5625.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=69&photo=Car-5626.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=70&photo=Car-5627.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=71&photo=Car-5628.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=72&photo=Car-5629.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=73&photo=Car-5630.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=74&photo=Car-5631.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=75&photo=Car-5632.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=76&photo=Car-5633.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=77&photo=Car-5634.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=78&photo=Car-5635.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=79&photo=Car-5636.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=80&photo=Car-5637.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=81&photo=Car-5638.jpg" | check_success
curl_post action.php "action=photo.assign&repo=car&racer=82&photo=Car-5639.jpg" | check_success
# curl_post action.php "action=photo.assign&repo=car&racer=83&photo=Car-7261.jpg" | check_success


## Once this runs, the file links in the database become invalid
## TODO rm -rf "$PHOTO_DIR" "$CAR_PHOTO_DIR"
