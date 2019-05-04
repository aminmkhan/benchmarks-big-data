# Husky Configuration for Cluster

<!-- MarkdownTOC levels="2,3,4,5" autoanchor=false autolink=true -->

- [Configuration file for Husky Master and Workers](#configuration-file-for-husky-master-and-workers)
- [Launching Husky Master](#launching-husky-master)
- [Machines Configuration](#machines-configuration)
    - [Scripts for Machine Configuration](#scripts-for-machine-configuration)
- [Launching Husky with `sbatch` on HPC Cluster](#launching-husky-with-sbatch-on-hpc-cluster)
- [Launching Husky with `qlogin` on HPC Cluster](#launching-husky-with-qlogin-on-hpc-cluster)
- [Running Husky Examples and Benchmarks](#running-husky-examples-and-benchmarks)
- [Parsing Husky Logs for Plots](#parsing-husky-logs-for-plots)
- [Useful Shell Commands](#useful-shell-commands)
    - [Setting up Husky on Cluster](#setting-up-husky-on-cluster)
    - [Getting the data](#getting-the-data)
- [Troubleshooting](#troubleshooting)
    - [`LogMessage` and `MailboxEventLoop` Failures](#logmessage-and-mailboxeventloop-failures)
    - [Worker configuration](#worker-configuration)
    - [Memory Allocation with PageRank](#memory-allocation-with-pagerank)
    - [Networking Issues](#networking-issues)

<!-- /MarkdownTOC -->

## Configuration file for Husky Master and Workers

Make sure [Husky installation](husky.md) is complete. Refer to [Husky's config guide] for details.

Husky's documentation provides the following example of configuration guide:

```bash
# Required
master_host=master
master_port=10086
comm_port=12306

# Worker information
[worker]
info=worker1:4
info=worker2:4
```

For example, here is a file to launch both master and worker on the local machine. You have to specify node's DNS or IP address for `master_host` and under `[worker] info`.

```bash
# Required
master_host=localhost
master_port=10086
comm_port=12306

# Optional
log_dir=$HOME/logs-husky
#hdfs_namenode=xxx.xxx.xxx.xxx
#hdfs_namenode_port=yyyyy

# For Master
serve=1

# Session for worker information
[worker]
info=localhost:4
info=localhost:2
```

You can create the file in `$HOME` folder or local Husky folder, with above settings.

```bash
mkdir -p $HOME/.husky
touch $HOME/.husky/husky-config.ini
```

## Launching Husky Master

```bash
export HUSKY_ROOT=$HOME/husky
cd $HUSKY_ROOT/release

# Build examples
make help
make -j4 PI
make -j4 PageRank
make -j4 BenchPageRank

# Launch the Master
./Master --help
./Master --conf=$HOME/.husky/husky-config.ini
./Master --conf=$HOME/.husky/husky-config.ini --log_dir=$HOME/logs-husky >> husky.log  2>&1 &

# Try examples
./PI --conf=$HOME/.husky/husky-config.ini
./PageRank --conf=$HOME/.husky/husky-config.ini
```

[Husky's config guide]: https://github.com/husky-team/husky/wiki/Config-How-to

## Machines Configuration

Husky provides some scripts which can be used with `mpirun` or `pssh` , and need a file with list of machines' IP addresses or hostnames.

```bash
# This points to a file, which should contains hostnames (one per line).
# E.g.,
#
# worker1
# worker2
# worker3
#
MACHINE_CFG=
```

Testing in interactive mode:

```bash
qlogin --account=nn9342k --nodes=2 --tasks-per-node=1
source /cluster/bin/jobsetup

echo $SLURM_SUBMIT_HOST
scontrol show hostnames $SLURM_JOB_NODELIST

hostname

./Master --master_host=c14-4 --master_port=10086 --comm_port=12306 --worker.info=c14-5:1 --serve=1

# check if ports connected
nc -vz c14-4 10086
nc -vz c14-4 12306
```

Running some Husky examples:

```
./PI --master_host=c14-4 --master_port=10086 --comm_port=12306 --worker.info=c14-5:1

./PageRank --master_host=c14-4 --master_port=10086 --comm_port=12306 --worker.info=c14-5:1 --input=$HOME/data/graph.txt --iter=2
```

### Scripts for Machine Configuration

We have included the script [`config-formatter.py`](../scripts/husky/config-formatter.py), which can be used to generate Husky's `config` file.

You can use [Slurm Environmental Variables] to get hostnames:

```bash
echo $SLURM_JOB_NODELIST
scontrol show hostnames $SLURM_JOB_NODELIST
```

[Slurm Environmental Variables]: https://www.glue.umd.edu/hpcc/help/slurmenv.html

## Launching Husky with `sbatch` on HPC Cluster

- You can use the scripts provided in [`scripts/husky/`](../scripts/husky/) folder to run batch jobs on the HPC cluster.
- Refer for details to the [`husky-pagerank.batch`](../scripts/husky/husky-pagerank.batch) script for using with `sbatch`.
- You can also use the [`pagerank-husky-local.sh`](../scripts/husky/pagerank-husky-local.sh) script for quick testing on local machine.

## Launching Husky with `qlogin` on HPC Cluster

With `qlogin` you run Husky examples manually interacting with all the individual nodes using `ssh`. The recommended approach is to instead use scripts provided for `sbatch`.

- Launch 3 nodes

    qlogin --account=nn9342k --nodes=3 --tasks-per-node=1

- Job setup

    source /cluster/bin/jobsetup

- Get nodes hostnames

    echo $SLURM_JOB_NODELIST
    scontrol show hostnames $SLURM_JOB_NODELIST

- Note down the nodes' hostnames

    c14-29
    c16-20
    c16-26

- Open separate shell and login to all the nodes (using same password as login node)

    ssh c14-29
    ssh c16-20
    ssh c16-26

- Update config file

```bash
# Required
master_host=localhost
master_port=10086
comm_port=12306

# Optional
log_dir=$HOME/logs-husky
hdfs_namenode=127.0.0.1
hdfs_namenode_port=80

# For Master
serve=1

# Session for worker information
[worker]
info=localhost:4
```

- You can use `iTerm` or `Teminal` applications on macOS to send commands to all open shells at the same time
    + Master launched in separate window.
    + All workers loaded in panes in all the rest.

- Here is a rundown on the commands I ran, your mileage may vary.

```bash
# get the master node
qlogin --account=nn9342k --nodes=1 --cpus-per-task=2 --mem-per-cpu=4G

# get worker nodes
qlogin --account=nn9342k --nodes=1 --ntasks-per-node=1 --mem-per-cpu=3936M --cpus-per-task=16

module load python2 gcc

source /cluster/bin/jobsetup
source ~/.bash_profile

# get hostsname, and update config files
hostname
echo $SLURM_JOB_NODELIST
scontrol show hostnames $SLURM_JOB_NODELIST


cd ~/husky/release
./Master --conf=husky-32-nodes.ini

./BenchPageRank --conf husky-32-nodes.ini --iter 50 --input nfs:///work/users/akhan/data/pagerank/web-indochina-2004-all.mtx

# Logging the commands executed, and the output from Husky
printf "\n\n./BenchPageRank --conf husky-32-nodes.ini --iter 200 --input nfs:///work/users/akhan/data/pagerank/Google_genGraph_20.txt \n" >> timings-32-nodes/timings.txt
./BenchPageRank --conf husky-32-nodes.ini --iter 200 --input nfs:///work/users/akhan/data/pagerank/Google_genGraph_20.txt >> timings-32-nodes/timings.txt  2>&1

# Kill Master before running example again
kill $(ps aux | grep 'Master --' | awk '{print $2}') 
./Master --conf local-1-node-1-worker.ini &> /dev/null &

./PageRank --conf ../release/local.ini --input nfs:///work/users/akhan/data/pagerank/Google_genGraph_20.txt --iter 3
```

[Heterogeneous Jobs]: https://slurm.schedmd.com/heterogeneous_jobs.html

## Running Husky Examples and Benchmarks

Run example programs.

```bash
cd $HUSKY_ROOT/release

./PI --help
./PageRank --help

./PI --conf=$HOME/.husky/husky-config.ini
```

## Parsing Husky Logs for Plots

- See [husky.ipynb](../plots/husky.ipynb) for a Python script to parse Husky's logs to generate `csv` file for easier plotting.

- See [plots.ipynb](../plots/plots.ipynb) for generating plots using `matplotlib` and `seaborn` in Python.

## Useful Shell Commands

### Setting up Husky on Cluster 

- Copy files to server or cluster using `rsync`:

```bash
rsync --dry-run -rtuv -l -e ssh --exclude 'cmake/' --exclude '.git/*' --exclude 'release/*' --include '*.sh' --include '*.ini' --exclude=.git ~/Github/husky akhan@abel.uio.no:husky/
```

- Or, copy files back from server to local machine, if needed:

```bash
rsync --dry-run -rtuv -l -e ssh --exclude='*' --include='*/' --include='*.sh' --include='*.ini'  akhan@abel.uio.no:husky/release ~/Github/husky/release
```

### Getting the data

- You can download the exported log files using `rsync`:

```bash
# Try first with --dry-run flag!
rsync -rtuv  -l -e ssh --exclude '$HOME' --exclude 'logs-husky' --exclude 'core.*' akhan@abel.uio.no:Documents/timings/* ~/Downloads/Benchmark-Data/timings-folder/.

```

- You can extract the running times in `csv` format using `grep`:

```bash
cat `find nodes* -iname '*.log' -print` | grep -i 'platform,' | head -n 1 > husky.csv
cat `find nodes* -iname '*.log' -print` | grep -i 'husky,' | sort >> husky.csv


cat `find . -iname '*.log' -print` | grep -i 'platform,' | head -n 1 > husky-pagerank-nodes-multiple.csv
cat `find . -iname '*.log' -print` | grep -i 'husky,' | sort >> husky-pagerank-nodes-multiple.csv
```

## Troubleshooting

### `LogMessage` and `MailboxEventLoop` Failures

Try not to use logging by skipping `--log_dir`, to avoid such errors. Such problems can occur because of port conflicts, when multiple workers are launched on the same node.

```bash
[FATAL 2019-03-20 12:52:23.989675  3870 mailbox.cpp:282] Assert failed: registered_mailbox_.find(thread_id) != registered_mailbox_.end()
("[ERROR] Local mailbox for " + std::to_string(thread_id) + " does not exist").c_str()
```

### Worker configuration

If the examples don't appear to finish or seem stuck, check the following:

- In the distributed environment, make sure to execute workers on all machines (same as specified in `config` file). 
- If `config` file specifies multiple workers, but if not all are running, then Husky may not work.
- The first line below `[worker]` in `config` file must not be commented out.

### Memory Allocation with PageRank

If you specify a large input file with insufficient memory, your program will get aborted:

```bash
./BenchPageRank --conf ../release/local.ini --input nfs:///work/users/akhan/data/pagerank/web-indochina-2004-all.mtx --iter 1 --hdfs_namenode NULL --hdfs_namenode_port 0

terminate called after throwing an instance of 'std::bad_alloc'
  what():  std::bad_alloc
Aborted
```

### Networking Issues

- Make sure Master is up on the given port:
    + `nc -vz localhost 10086`
- Communication port is normally not active, you will get `connection refused` error:
    + `nc -vz localhost 12306`
- When the workers are running an example, then communication port will be used.
    + `nc -vz localhost 12306`
 