from django.contrib import admin

# Register your models here.

from ldqm_db.models import Run, AMC, GEB, VFAT, HWstate, SystemState
admin.site.register(Run)
admin.site.register(AMC)
admin.site.register(GEB)
admin.site.register(VFAT)
admin.site.register(HWstate)
admin.site.register(SystemState)
