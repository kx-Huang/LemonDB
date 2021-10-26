# LemonDB Test Environment

# setup

- copy from remote server `/opt/lemondb/sample_dump` to local `test/ref_dump`

```bash
    scp -r VE482:/opt/lemondb/sample_dump test/ref_dump
```

# build, benchmark and diff

- Build LemonDB to `build/lemondb`
- Dump output to `test/sample_dump`
- Diff with `test/ref_dump`
- Test runtime

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
-- Build files have been written to: ~/VE482_p2/build
[100%] Built target lemondb
=================================================================================
 Running lemondb...
=================================================================================

  query name             time
 -----------------------------------
  few_insert_delete      10.696

=================================================================================
[Error] output doesn't match for "sample_dump/few_insert_delete_dump_fTable0.tbl"
[Log] 3,7657c3,7594 < r0 63551 -22789 9512 -23499 38954 62633 -63108 -15454 -26513 -14525 -55907 2539 21614 32632 -22417 7276 -6819 57566 6610 24807 < r1 63551 -22789 -26737 -23499 38954 62633 -63108 -15454 -26513 -14525 -55907 2539 21614 32632 22653 -6819 7276 57566 13805 24807
=================================================================================
```
