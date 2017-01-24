//Load the YouTube iframe API
var tag = document.createElement('script');

tag.src = "https://www.youtube.com/iframe_api";
var firstScriptTag = document.getElementsByTagName('script')[0];
firstScriptTag.parentNode.insertBefore(tag, firstScriptTag);

var player;
function onYouTubeIframeAPIReady() {
    //Create an iframe for the video
    player = new YT.Player('theShellPromoVideo', {
        height: '100%',
        width: '100%',
        videoId: 'oHtIe7MS9ig',
        events: {
            'onStateChange': onPlayerStateChange
        }
    });
}

function onPlayerStateChange(event) {
    if (event.data == YT.PlayerState.ENDED) {
        //Once finished playing, toggle the video visibility
        promoVideo();
    }
}

function playVideo() {
    player.playVideo();
}

function stopVideo() {
    player.stopVideo();
}
