console.log( "simpleHist01.js : " );
// json file stored in same folder, absolute address can be used as well
//JSROOT.NewHttpRequest("hpx.json", 'object', function(obj) {
JSROOT.NewHttpRequest("BeamProfile.json", 'object', function(obj) {
   JSROOT.draw("drawing", obj, "hist");
}).send();
