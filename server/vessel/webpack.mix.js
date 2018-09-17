let mix = require('laravel-mix');

/*
 |--------------------------------------------------------------------------
 | Mix Asset Management
 |--------------------------------------------------------------------------
 |
 | Mix provides a clean, fluent API for defining some Webpack build steps
 | for your Laravel application. By default, we are compiling the Sass
 | file for the application as well as bundling up all the JS files.
 |
 */

mix.js('resources/assets/js/app.js', 'public/js/app.js')
	.js('resources/assets/js/vue.js', 'public/js/vue.js')
	.sass('resources/assets/sass/app.scss', 'public/css')
	.copy('resources/assets/semantic/dist/tablesort.min.js','public/js/tablesort.min.js')
	.copy('resources/assets/semantic/dist/semantic.min.js','public/js/semantic.min.js')
	.copy('resources/assets/semantic/calendar/calendar.min.js','public/js/calendar.min.js')
	.copy('resources/assets/semantic/dist/themes/', 'public/css/themes/', false)
	.copyDirectory('resources/assets/images', 'public/images');

mix.autoload({
 'jquery': ['$','jQuery']
});

mix.styles([
	'resources/assets/semantic/dist/semantic.min.css',
	'resources/assets/semantic/calendar/calendar.min.css'
], 'public/css/semantic.min.css');

mix.styles([
	'resources/assets/vessel/vessel.css',
], 'public/css/vessel.css');
