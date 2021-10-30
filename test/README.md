# LemonDB Test Environment

## unit test

### build, input and run your query

- Build LemonDB to `build/lemondb`
- Unit test files:
    - generate sample random table: `unit_test/unit.tbl`
    - store input query: `unit_test/unit.tbl`
    - log stdout: `unit_test/unit.stdout`
    - log stderr: `unit_test/unit.stderr`

### Usage
#### Generate random 5x5 table and enter interaction mode
```bash
    ./unit.sh
```

#### Read from argument
- Read table from my path and enter interaction mode
```bash
    ./unit.sh unit.tbl
```
- Read query from my path (table must be specified) and directly output stdout and stderr
```bash
    ./unit.sh unit.tbl unit.query
```

#### Handle `permission denied`
If `permission denied`, try following command to grant the script execution permission

```bash
    chmod +x ./unit.sh
```

### sample output
#### interaction mode
```log
=================================================================================
Unit test
=================================================================================
[Running] building lemondb...
-- Configuring done
-- Generating done
-- Build files have been written to: ~/VE482_p2/build
[100%] Built target lemondb
[Success] done building lemondb
=================================================================================
[Running] generating sample table...

test 6
        KEY     c0      c1      c2      c3      c4
        r0      2       2       4       1       2
        r1      5       1       3       2       3
        r2      1       0       3       3       1
        r3      4       5       2       0       0
        r4      1       5       2       1       1

=================================================================================
[Query] COUNT ( ) FROM test;
ANSWER = 5
=================================================================================
[Query] MAX ( c0 c1 ) FROM test WHERE ( c3 >= 2 );
ANSWER = ( 5 1 )
=================================================================================
[Query] q
exit
=================================================================================
```

### direct output mode
```log
=================================================================================
Unit test
=================================================================================
[Running] building lemondb...
-- Configuring done
-- Generating done
-- Build files have been written to: ~/VE482_p2/build
[100%] Built target lemondb
[Success] done building lemondb
=================================================================================
[Running] Reading table from "unit.tbl"...
[Running] Reading query from "unit.query"...
=================================================================================
[Log] stdout:
1
2
Affected 5 rows.
3
=================================================================================
[Log] stderr:
lemondb: info: running in 1 threads
=================================================================================
```

## integration test

### setup

- copy from remote server `/opt/lemondb/sample_dump` to local `test/ref_dump`
- copy from remote server `/opt/lemondb/sample_stdout` to local `test/ref_stdout`

```bash
    cd test
    scp -r Remote:/opt/lemondb/sample_dump ref_dump
    scp -r Remote:/opt/lemondb/sample_stdout ref_stdout
```

### build, run, benchmark and diff

- Build LemonDB to `build/lemondb`
- Save output files to `test/sample_dump` and `test/sample_stdout`
- Diff with `test/ref_dump` and `test/ref_stdout`
- Record runtime

### Usage
```bash
    cd test
    ./integration.sh
```

If `permission denied`, try following command to grant the script execution permission

```bash
    chmod +x ./integration.sh
```

### sample output

```log
=================================================================================
 Building lemondb...
=================================================================================
-- Configuring done
-- Generating done
-- Build files have been written to: ~/VE482_p2/build
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
