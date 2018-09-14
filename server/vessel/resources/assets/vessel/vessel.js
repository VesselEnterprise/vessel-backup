(function($,window) {

	$(document).ready(function() {

		var timeoutId = null;

		$('#search-text').keyup(function(e) {
			clearTimeout(timeoutId);
			timeoutId = setTimeout( () => search(e.target.value), 500 );
		});

	});

	function search(text) {

		$.get('search/' + text, displaySearchResults);

	}

	function displaySearchResults(data) {
		console.log(data);
	}

})($,window);
