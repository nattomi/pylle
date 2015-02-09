$(function() {
    var ws = new WebSocket("ws://aero.nck.ggki.hu:8888/");
    var $placeholder = $('#placeholder');
    var datalen = 30*60*1000; // time in milliseconds
    var plot = null;
    var series = { 
        label: "Potential gradient", 
        lines: { 
            show: true, 
            fill: false 
        }, 
        points: { 
            show:false
        },
	shadowSize: 0,
        data: [] 
    };
    var last_x = 0;
  
    ws.onmessage = function(evt) {
	var d = $.parseJSON(evt.data);
	if(d.x - last_x >= 5) {
	    last_x = d.x;
	    var xlim = {"min":last_x - datalen,
			"max":last_x};
	    series.data.push([d.x,d.y]);
	    while (series.data[0][0] < xlim.min) {
		series.data.shift();
	    }
	    
	    if(plot) { // Update the plot
		plot.setData([series]);
		plot.getOptions().xaxes[0].min = xlim.min;
		plot.getOptions().xaxes[0].max = xlim.max;
		plot.setupGrid(); 
		plot.draw(); 
	    } else { // Create the plot if it's not there already 
		plot = $.plot($placeholder, [series], {
		    xaxis: {
			mode: "time",
			timeformat: "%H:%M:%S",
			tickSize: [5, "minute"],
			min: xlim.min,
			max: xlim.max
		    },
		    yaxis: { 
			min: -1024, 
			max: 1023 
		    } 
		});
		plot.draw(); 
	    } 
	}
    }  
});
