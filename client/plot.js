$(function() {
    var ws = new WebSocket("ws://aero.nck.ggki.hu:8888/");
    var m = -0.4352874 // scale factor (slope)  
    var b = 4.3528735 // scale factor (intersect) 
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
    var xlim;

    function createPlot(plot) { // for creating a graph for the first time
	plot = $.plot($placeholder, [series], {
	    xaxis: {
		mode: "time",
		timeformat: "%H:%M:%S",
		tickSize: [5, "minute"],
		min: xlim.min,
		max: xlim.max
	    },
	    yaxis: { 
		min: -500, 
		max:  500,
		tickSize: 50
	    } 
	});
	plot.draw();
    }

    function updatePlot(plot) { // for updating an already existing graph
	plot.setData([series]);
	plot.getOptions().xaxes[0].min = xlim.min;
	plot.getOptions().xaxes[0].max = xlim.max;
	plot.setupGrid(); 
	plot.draw();
    }

    function updateSeries(d) { // for updating series to be plotted
	last_x = d.x;
	xlim = {"min":last_x - datalen,
		"max":last_x};
	series.data.push([d.x,m*d.y+b]);
	while (series.data[0][0] < xlim.min) {
	    series.data.shift();
	}
    }
  
    ws.onmessage = function(evt) {
	var d = $.parseJSON(evt.data);
	/* at first we make sure that the received data is  
	   actually "newer" than what we had so far */
	if(d.x - last_x >= 5) {  
	    updateSeries(d);
	    /* decide whether plot should be
	       created or updated */
	    plot ? updatePlot() : createPlot(); 
	}
    }  
});    