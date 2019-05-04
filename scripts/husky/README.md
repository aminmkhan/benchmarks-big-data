# Benchmarks for Husky

Refer to [Husky under documentation](../../docs/husky.md).

- Copy this file to Husky's benchmark folder and .

```bash
rsync -rtuv -l -e ssh --exclude '.git/*' ~/GitHub/husky-pagerank/benchmarks/husky/ akhan@abel.uio.no:husky/benchmarks
```

- Follow the [instructions](../../docs/husky.md) to compile it.

```bash
cd $HUSKY_ROOT/release
make -j 4 BenchPageRank
```
