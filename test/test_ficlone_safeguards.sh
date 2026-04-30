# shellcheck shell=sh
# Sourced by test_btrfs_clone.sh and test_bcachefs.sh.
# Requires: FS_MOUNTPOINT, FS_RMW_CMD, FS_WASTE_DIR

# --- Test: no orphan file in waste when source unlink fails ---
echo "== Test: no orphan in waste when source file unlink fails"
FS_RO_DIR="$FS_MOUNTPOINT/ro_dir"
chmod 0755 "$FS_RO_DIR" 2>/dev/null || true
rm -rf "$FS_RO_DIR"
mkdir "$FS_RO_DIR"
touch "$FS_RO_DIR/file"
chmod 0555 "$FS_RO_DIR"
$FS_RMW_CMD "$FS_RO_DIR" || true
chmod 0755 "$FS_RO_DIR"
test ! -f "$FS_WASTE_DIR/files/ro_dir/file"
test -f "$FS_RO_DIR/file"
rm -rf "$FS_RO_DIR"

# --- Test: no orphan symlink in waste when source symlink unlink fails ---
echo "== Test: no orphan symlink in waste when source symlink unlink fails"
chmod 0755 "$FS_RO_DIR" 2>/dev/null || true
rm -rf "$FS_RO_DIR"
mkdir "$FS_RO_DIR"
ln -s nonexistent "$FS_RO_DIR/link"
chmod 0555 "$FS_RO_DIR"
$FS_RMW_CMD "$FS_RO_DIR" || true
chmod 0755 "$FS_RO_DIR"
test ! -L "$FS_WASTE_DIR/files/ro_dir/link"
test -L "$FS_RO_DIR/link"
rm -rf "$FS_RO_DIR"
