# shellcheck shell=sh
# Sourced by test_btrfs_clone.sh and test_bcachefs.sh.
# Requires: FS_MOUNTPOINT, FS_RMW_CMD, FS_WASTE_DIR

# do_ficlone_dir copies the whole tree first, then removes the source
# (coreutils mv semantics). A clone failure rolls back cleanly (source
# untouched, nothing left in the waste); a source-removal failure keeps the
# copy in the waste so data is never lost.

# --- Test: clone failure rolls back cleanly (source intact, no waste copy) ---
# A FIFO can't be cloned, so clone_tree fails and the partial copy is removed.
# The dir is also read-only so the rename fallback (to another waste) can't
# move it either -- leaving the source fully intact and the waste clean.
echo "== Test: an un-clonable entry rolls back the directory copy"
FS_FIFO_DIR="$FS_MOUNTPOINT/fifo_dir"
chmod 0755 "$FS_FIFO_DIR" 2>/dev/null || true
rm -rf "$FS_FIFO_DIR" "$FS_WASTE_DIR/files/fifo_dir"
mkdir "$FS_FIFO_DIR"
touch "$FS_FIFO_DIR/regular"
mkfifo "$FS_FIFO_DIR/pipe"
chmod 0555 "$FS_FIFO_DIR"
$FS_RMW_CMD "$FS_FIFO_DIR" || true
chmod 0755 "$FS_FIFO_DIR"
test -f "$FS_FIFO_DIR/regular"
test -p "$FS_FIFO_DIR/pipe"
test ! -d "$FS_WASTE_DIR/files/fifo_dir"
rm -rf "$FS_FIFO_DIR" "$FS_WASTE_DIR/files/fifo_dir"

# --- Test: copy kept in waste when the source file can't be removed ---
echo "== Test: copy kept in waste when source file unlink fails"
FS_RO_DIR="$FS_MOUNTPOINT/ro_dir"
chmod 0755 "$FS_RO_DIR" 2>/dev/null || true
rm -rf "$FS_RO_DIR" "$FS_WASTE_DIR/files/ro_dir"
mkdir "$FS_RO_DIR"
touch "$FS_RO_DIR/file"
chmod 0555 "$FS_RO_DIR"
$FS_RMW_CMD "$FS_RO_DIR" || true
chmod 0755 "$FS_RO_DIR"
test -f "$FS_WASTE_DIR/files/ro_dir/file"
test -f "$FS_RO_DIR/file"
rm -rf "$FS_RO_DIR" "$FS_WASTE_DIR/files/ro_dir"

# --- Test: copy kept in waste when the source symlink can't be removed ---
echo "== Test: copy kept in waste when source symlink unlink fails"
chmod 0755 "$FS_RO_DIR" 2>/dev/null || true
rm -rf "$FS_RO_DIR" "$FS_WASTE_DIR/files/ro_dir"
mkdir "$FS_RO_DIR"
ln -s nonexistent "$FS_RO_DIR/link"
chmod 0555 "$FS_RO_DIR"
$FS_RMW_CMD "$FS_RO_DIR" || true
chmod 0755 "$FS_RO_DIR"
test -L "$FS_WASTE_DIR/files/ro_dir/link"
test -L "$FS_RO_DIR/link"
rm -rf "$FS_RO_DIR" "$FS_WASTE_DIR/files/ro_dir"
