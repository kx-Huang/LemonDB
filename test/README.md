# LemonDB Test Environment

# setup

- copy from remote server `/opt/lemondb/sample_dump` to local `test/ref_dump`
- copy from remote server `/opt/lemondb/sample_stdout` to local `test/ref_stdout`

```bash
    cd test
    scp -r Remote:/opt/lemondb/sample_dump ref_dump
    scp -r Remote:/opt/lemondb/sample_stdout ref_stdout
```

# build, run, benchmark and diff

- Build LemonDB to `build/lemondb`
- Save output files to `test/sample_dump` and `test/sample_stdout`
- Diff with `test/ref_dump` and `test/ref_stdout`
- Record runtime

```bash
    cd test
    ./tester.sh
```

# sample output

```log
=================================================================================
 Building lemondb...
=================================================================================
-- Configuring done
-- Generating done
-- Build files have been written to: /Users/michaelhuang/Desktop/VE482_p2/build
[100%] Built target lemondb
=================================================================================
 Running lemondb...
=================================================================================
  query name             time
 -----------------------------------
  few_insert_delete      9.629
=================================================================================
[Error] output doesn't match for "sample_stdout/few_insert_delete.out"
[Log] 6a7
> ANSWER = 941
=================================================================================
  SUM                    9.629
=================================================================================
```
