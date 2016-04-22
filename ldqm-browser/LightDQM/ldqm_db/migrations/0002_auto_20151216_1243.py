# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('ldqm_db', '0001_initial'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='run',
            name='Period',
        ),
        migrations.RemoveField(
            model_name='run',
            name='Station',
        ),
    ]
