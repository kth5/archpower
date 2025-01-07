# ansible

## ansible new upstream releases silently depends on specific versions of ansible-core

The `ansible` upstream source (which provides community modules/collections for Ansible) is developed in correlation with `ansible-core` (for compatibility matters).  
However, they do not share a similar/synced version scheme nor have any mechanism that would clearly indicate this when packaging it; apart from a note in their [changelogs](https://forum.ansible.com/t/release-announcement-ansible-community-package-10-3-0/7821) (e.g. from the changelog linked: "Ansible 10.3.0 depends on ansible-core 2.17.3 [...]").

In other words, `ansible` new releases *silently* depend on the latest `ansible-core` version available at the time.  
To put it simply, `ansible` new releases should always be built against an up to date version of `ansible-core`. Building new `ansible` releases against outdated versions of `ansible-core` would inevitably create incompatibility issues for some community modules.

Fortunately, upstream usually syncs the releases of new `ansible` and `ansible-core` versions. They are usually released a few hours apart (with `ansible-core` always being released first).

As a matter of precaution, it is appropriate to maintain both `ansible` and `ansible-core` as one package group (e.g. package, release and move new versions of those packages accross repositories together).
