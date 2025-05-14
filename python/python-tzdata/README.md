# python-tzdata

---
**NOTE**

Since Python 3.9, this library is no longer needed on Arch Linux.
The [zoneinfo module in the Python standard library] automatically supports _zoneinfo data_ as provided by [the tzdata package].
Only if system-provided _zoneinfo data_ is not available, the _zoneinfo module_ automatically falls back to using the _tzdata Python package_.

Python upstreams may rely specifically on the _tzdata Python package_ to work out-of-the-box on all systems (also those not providing _zoneinfo data_).
However, on Arch Linux we do not want to maintain the _tzdata Python package_ (because we have [the tzdata package]) and specific use of it must be removed from upstream projects by patching them.

---

[zoneinfo module in the Python standard library]: https://docs.python.org/3/library/zoneinfo.html
[the tzdata package]: https://archlinux.org/packages/core/x86_64/tzdata/
