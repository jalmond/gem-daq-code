# coding: utf-8
from django.conf.urls import patterns, include, url
from webdaq.views import *
from django.core.urlresolvers import reverse_lazy

urlpatterns = patterns('',
    url(r'^$', gemsupervisor),
)
