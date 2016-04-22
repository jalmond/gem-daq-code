# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import datetime
from django.utils.timezone import utc


class Migration(migrations.Migration):

    dependencies = [
        ('ldqm_db', '0002_auto_20151216_1243'),
    ]

    operations = [
        migrations.AddField(
            model_name='run',
            name='Period',
            field=models.CharField(default=datetime.datetime(2015, 12, 16, 12, 46, 33, 792972, tzinfo=utc), max_length=10),
            preserve_default=False,
        ),
        migrations.AddField(
            model_name='run',
            name='Station',
            field=models.CharField(default='TAMU test', max_length=10),
            preserve_default=False,
        ),
    ]
