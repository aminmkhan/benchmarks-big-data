# Husky: Benchmarking on Cluster

<!-- MarkdownTOC levels="2,3,4,5" autoanchor=false autolink=true -->

- [PageRank Benchmark](#pagerank-benchmark)
    - [Compiling PageRank](#compiling-pagerank)
    - [Running examples](#running-examples)
    - [Input File Format](#input-file-format)

<!-- /MarkdownTOC -->

## PageRank Benchmark

Make sure [Husky installation](husky.md) and [Husky configuration](husky-config.md) is complete.

### Compiling PageRank

Copy the [`pagerank.cpp`](../scripts/husky/pagerank.cpp) to `$HUSKY_ROOT/benchmarks` and run the following commands (after changing the file paths):


```bash
cp $SADDLEBAG_INSTALL/benchmarks/husky/pagerank.cpp $HUSKY_ROOT/benchmarks
cd $HUSKY_ROOT/release
rm ./BenchPageRank 
make -j8 BenchPageRank
```

### Running examples

- To quickly compile:

```bash
rm ./BenchPageRank 
make -j8 BenchPageRank &> /dev/null
```

- Run example:

```bash
# Kill running Husky Master instance
kill $(ps aux | grep 'Master --' | awk '{print $2}')

# Launch Master
./Master --conf local.ini &> /dev/null &

./BenchPageRank --conf local.ini --input nfs:///usit/abel/u1/akhan/nobackup/opt/husky/benchmarks/simple_graph.txt --iter 10  
```

- To kill Master process:

```bash
kill $(ps aux | grep 'Master --' | awk '{print $2}')
```

- You can also use run the examples using the included [script](../scripts/husky/husky-pagerank.batch) like this:

```bash
sbatch --nodes=4 ~/husky/benchmarks/husky-pagerank.batch
```

### Input File Format

See [graph file formats](datasets.html#graph-file-formats) for details on the input files for PageRank example.

Husky expects the input on the command line as either `hdfs://` or `nfs://` links. You can pass a local file by prefixing its absolute path with the `nfs://`.

- For example:
    + `/work/users/akhan/data/pagerank/Google_genGraph_20.txt` can be passed as
    + `nfs:///work/users/akhan/data/pagerank/Google_genGraph_20.txt`
