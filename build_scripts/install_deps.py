from common import *

os.chdir(EXTERNAL_DIR)

PKGS = os.environ.get('PKGS')

if not PKGS:
    PKGS = env('PKGS', 'all')

#print("PKGS: %s" % PKGS)

print("PKGS: %s" % PKGS)

ninja_template = read_file(f'{SCRIPT_DIR}/install_deps_{OS_LOWER_CASE}.ninja.in')
write_file(f'install_deps_{OS_LOWER_CASE}.ninja', ninja_template.format(**ENV_PARAMS))
for key, val in ENV_PARAMS.items():
    print(f'{key} = {val}')
shell(f'ninja -v -j 1 -f install_deps_{OS_LOWER_CASE}.ninja {PKGS}')
