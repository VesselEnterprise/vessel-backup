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

mix.autoload({
 jquery: ['$', 'window.jQuery', 'jQuery']
});

mix.js('resources/assets/js/app.js', 'public/js')
	.js('resources/assets/js/vue.js', 'public/js')
	.sass('resources/assets/sass/app.scss', 'public/css')
	.copy('resources/assets/semantic/dist/semantic.min.js','public/js/semantic.min.js')
	.copy('resources/assets/semantic/dist/themes/', 'public/css/themes/', false)
	.copyDirectory('resources/assets/images', 'public/images');

mix.styles([
	'resources/assets/semantic/dist/semantic.min.css'
], 'public/css/semantic.min.css');
