#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#!/usr/bin/env python

import sys

config_template = """
# Required
master_host=<MASTER>
master_port=10086
comm_port=12306

# Optional
log_dir=logs-husky
hdfs_namenode=localhost
hdfs_namenode_port=10

# For Master
serve=1

# Session for worker information
[worker]

"""
# info=<WORKER>:<THREADS>

def generate_config(hosts_file='hosts.ini', config_file='config.ini', threads=16, template=config_template):
    """Parse host file to generate config
    """

    debug = False

    # open file and read the content in a list
    with open(hosts_file, 'r') as file_handle:
        hosts = [h.strip() for h in file_handle.readlines()]

    if len(hosts) == 0:
        return False

    for h in hosts[:]:
        if len(h) == 0:
            hosts.remove(h)

    # hosts.sort()

    config = template.strip() + '\n'

    first = True
    for h in hosts:
        if first:
            config = config.replace('<MASTER>', h)
            config += ('info=%s:%i\n' % (h, threads-1))
            first = False
        else:
            config += ('info=%s:%i\n' % (h, threads))

    with open(config_file, 'w+') as file_handle:
        file_handle.write(config)

    if debug:
        print(hosts)
        print(config)

    return True


def main(argv):
    """Program entry point.

    :param argv: command-line arguments
    :type argv: :class:`list`
    """

    hosts_file = r'hosts.ini'
    config_file = r'config.ini'

    if len(argv) > 1:
        hosts_file = argv[1]

    if len(argv) > 2:
        config_file = argv[2]

    return generate_config(hosts_file, config_file)


if __name__ == '__main__':

    main(sys.argv)
