# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='AMC',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('BoardID', models.CharField(max_length=30)),
                ('Type', models.CharField(max_length=30)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='GEB',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('Type', models.CharField(max_length=30)),
                ('ChamberID', models.CharField(max_length=30)),
            ],
            options={
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Run',
            fields=[
                ('id', models.AutoField(verbose_name='ID', serialize=False, auto_created=True, primary_key=True)),
                ('Name', models.CharField(max_length=50)),
                ('Type', models.CharField(max_length=10)),
                ('Number', models.CharField(max_length=10)),
                ('Date', models.DateField()),
                ('Period', models.CharField(max_length=10)),
                ('Station', models.CharField(max_length=10)),
                ('amcs', models.ManyToManyField(to='ldqm_db.AMC')),
            ],
            options={
                'ordering': ['Date'],
            },
            bases=(models.Model,),
        ),
        migrations.AddField(
            model_name='amc',
            name='gebs',
            field=models.ManyToManyField(to='ldqm_db.GEB'),
            preserve_default=True,
        ),
    ]
