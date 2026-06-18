install_man('rmw.1')

# The committed man page is generated from MANUAL.md at the repo root with
# go-md2man. To edit the man page, edit MANUAL.md and run
# `ninja -C <builddir> regen-man` to refresh rmw.1, then commit both. Normal
# builds just install the committed rmw.1, so go-md2man is not a build
# requirement (distros without it are unaffected).
go_md2man = find_program('go-md2man', required: false)
if go_md2man.found()
  run_target(
    'regen-man',
    command: [
      go_md2man,
      '-in',
      meson.project_source_root() / 'MANUAL.md',
      '-out',
      meson.current_source_dir() / 'rmw.1',
    ],
  )
endif
