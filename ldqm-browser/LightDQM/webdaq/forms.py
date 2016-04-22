from django import forms
#from django.forms.extras.widgets import Select

AMC13CHOICES = ((83,83),(43,43),(170,170))
AMCCHOICES = ((1,"1"),(2,2),(3,3),(4,4),(5,5),(6,6),(7,7),(8,8),(9,9),(10,10),(11,11),(12,12))
TRIGGERCHOICES = ((1,"local"),(2,"external"))
VERBOSITY = ((1,1),(2,2),(3,3),(4,4),(5,5),(6,6),(7,7),(8,8),(9,9))
class ConfigForm(forms.Form):
  amc13_choice = forms.ChoiceField(widget = forms.Select(attrs={'class':'form-control','style':'width:auto'}), choices=AMC13CHOICES)
  #amc_list = forms.ChoiceField(widget = forms.Select(attrs={'class':'form-control','style':'width:auto'}), choices=AMCCHOICES)# currently only one GLIB is supported, multiple selection will be implemented later
  amc_list = forms.MultipleChoiceField(widget = forms.SelectMultiple(attrs={'class':'form-control','style':'width:auto'}), choices=AMCCHOICES)
  trigger_type = forms.ChoiceField(widget = forms.Select(attrs={'class':'form-control','style':'width:auto'}), choices=TRIGGERCHOICES)
  trigger_rate = forms.IntegerField(widget = forms.NumberInput(attrs={'style':'width:80px'}),initial=10)
  verbosity = forms.ChoiceField(widget = forms.Select(attrs={'class':'form-control','style':'width:auto'}), choices=VERBOSITY)
