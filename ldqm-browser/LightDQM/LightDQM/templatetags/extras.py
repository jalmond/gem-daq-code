from django.apps import AppConfig
from django import template

register = template.Library()
@register.filter(name='get_index')
def get_index(list, index):
    return list[int(index)]

@register.filter(name='get_first')
def get_first(list, index):
    return list[int(index)][0]

@register.filter(name='get_second')
def get_second(list, index):
    z = int(index) - 1
    # print "get_second index=",z
    return list[z][1]
