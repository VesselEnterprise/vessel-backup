(function($,window) {

	$(document).ready(function() {

		$('.ui.search').search({
			type : 'category',
			minCharacters : 3,
			apiSettings: {
				url: '/search/{query}',
				onResponse: handleSearchResponse
			}
		});

	});

	function handleSearchResponse(searchResponse) {

		var response = {
			results: {}
		};

		var itemCount = 0;

		$.each(searchResponse, function(index, objs) {

			if ( objs.length > 0 ) {

				var itemInterface = getSearchInterface(index);
				var categoryName = itemInterface.category || 'Unknown';

				if ( response.results[categoryName] === undefined ) {
					response.results[categoryName] = {name: categoryName, results: []};
				}

				for( var i=0; i < objs.length; i++ ) {
					console.log(JSON.stringify(objs[i]));
					response.results[categoryName].results.push({
						title: getSearchItemTitle(objs[i], index),
						description: objs[i][itemInterface.descrKey],
						url: itemInterface.route + '/' + objs[i][itemInterface.key]
					});
					console.log('Added result');
				}

				itemCount++;

			}

		});

		return response;

	}

	function getSearchInterface(objectKey) {

		var categoryMap = {};
		categoryMap.users = {category: 'User', key: 'user_id', route: '/user', descrKey: 'title'};
		categoryMap.files = {category: 'File', key: 'file_id', route: '/file', descrKey: 'file_type'};
		categoryMap.settings = {category: 'Setting', key: 'setting_id', route: '/setting', descrKey: 'setting_name'};
		categoryMap.providers = {category: 'StorageProvider', key: 'provider_id', route: '/storage', descrKey: 'server'};
		categoryMap.clients = {category: 'Client', key: 'client_id', route: '/client', descrKey: 'ip_address'};

		return categoryMap[objectKey];

	}

	function getSearchItemTitle(item, key) {

		switch(key) {
			case 'users':
				return item.first_name + ' ' + item.last_name;
			case 'files':
				return item.file_name;
			case 'settings':
				return item.display_name;
			case 'providers':
				return item.provider_name;
			case 'clients':
				return item.client_name;
			default:
				return 'Unknown';
		}

		return 'Unknown';

	}

})($,window);
