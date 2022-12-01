from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add bmi160 src files.
src += Glob('sensor_bosch_bmx160.c')
src += Glob('libraries/bmi160.c')
src += Glob('libraries/bmm150.c')

# add bmi160 include path.
path  = [cwd, cwd + '/libraries']

# add src and include to group.
group = DefineGroup('bmi160', src, depend = ['PKG_USING_BMI160_BMX160'], CPPPATH = path)

Return('group')
