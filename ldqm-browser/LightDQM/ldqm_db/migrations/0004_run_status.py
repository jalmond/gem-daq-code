# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('ldqm_db', '0003_auto_20151216_1246'),
    ]

    operations = [
        migrations.AddField(
            model_name='run',
            name='Status',
            field=models.BooleanField(default=False),
            preserve_default=True,
        ),
    ]
