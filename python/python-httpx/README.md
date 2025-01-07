# python-httpx

## Co-developed with python-httpcore

`python-httpx` is co-developed with `python-httpcore` (upstream wise) and are often tied.  
In other words, latest `python-httpx` usually expects latest `python-httpcore`. As a matter of precaution, they should be maintained as one package group (e.g. package, release and move new versions of those packages accross repositories together).

## Frequent introduction of breaking changes

`python-httpx` often introduces breaking changes. As a matter of precaution, it is advised to rebuild / test reverse dependencies (meaning packages that depend on `python-httpx`) against new `python-httpx` upstream releases. A related ToDo task can opened, e.g. <https://archlinux.org/todo/python-httpx-028-rebuild/>.
