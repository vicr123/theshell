var animTimeout;

function promoVideo() {
    var videoEl = document.getElementById("theshellPromo");
    if (videoEl.style.height == "calc(100vh - 48px)") {
        videoEl.style.height = "0px";
        stopVideo();
    } else {
        videoEl.style.height = "calc(100vh - 48px)";
        
        animTimeout = setInterval(function() {
            document.getElementById("theshellPromo").scrollIntoView(false);
            timeTimeout = setTimeout(scrollToBottomOfPromo, 20);
        }, 20);
        setTimeout(function() {
            clearInterval(animTimeout);
        }, 1000);
        playVideo();
    }
}
