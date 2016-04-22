jQuery(document).ready(function($) {
  $('#myCarousel').carousel({
          interval: false
  });
  
  //Handles the carousel thumbnails
  $('[id^=carousel-selector-]').click(function () {
    var id_selector = $(this).attr("id");
    try {
      var id = /-(\d+)$/.exec(id_selector)[1];
      console.log(id_selector, id);
      jQuery('#myCarousel').carousel(parseInt(id));
    } catch (e) {
      console.log('Regex failed!', e);
    }
  });
  // When the carousel slides, auto update the text
  $('#myCarousel').on('slid.bs.carousel', function (e) {
    var id = $('.item.active').data('slide-number');
    $('#carousel-text').html($('#slide-content-'+id).html());
  });
});

jQuery(document).ready(function($) {
    $(".clickable-row").click(function() {
        window.document.location = $(this).data("href");
    });
});

$("#table tr").click(function(){
   $(this).addClass('selected').siblings().removeClass('selected');    
   var runN=$(this).find('td').eq(0).html();
   var runType=$(this).find('td').eq(1).html();
   var ref=window.location.protocol;
   ref+="//";
   ref+=window.location.host;
   ref+="/main/";
   ref+=runType;
   ref+="/run/";
   ref+=runN;
   window.document.location = ref;
});

//$(document).ready(function() {
//        $("#gebButton1").click(function(){
//        var ref="http://localhost:8000";
//        ref += window.location.pathname;
//        ref += $("#gebButton1").text();
//        window.document.location = ref;
//                        }); 
//});
//$(document).ready(function() {
//        $("#gebButton2").click(function(){
//        var ref="http://localhost:8000";
//        ref += window.location.pathname;
//        ref += $("#gebButton2").text();
//        window.document.location = ref;
//                        }); 
//});
