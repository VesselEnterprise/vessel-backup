<!DOCTYPE html>
<html lang="{{ str_replace('_', '-', app()->getLocale()) }}">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1.0">

    <!-- CSRF Token -->
    <meta name="csrf-token" content="{{ csrf_token() }}">

    <title>{{ config('app.name', 'Laravel') }}</title>

    <!-- Scripts -->
    <script src="{{ asset('js/app.js') }}"></script>
		<script src="{{ asset('js/vue.js') }}" defer></script>

		<!-- Semantic UI -->
		<link rel="stylesheet" type="text/css" href="{{ asset('css/semantic.min.css') }}">
		<!-- <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script> -->
		<!-- <script src="{{ asset('js/semantic.min.js') }}"></script> -->

		<!-- Init Semantic Controls -->
		<script>
			$(document).ready( function() {
				$('.ui.dropdown').dropdown();
				$('.ui.checkbox').checkbox();
			});
		</script>

    <!-- Fonts -->
    <link rel="dns-prefetch" href="https://fonts.gstatic.com">
    <link href="https://fonts.googleapis.com/css?family=Raleway:300,400,600" rel="stylesheet" type="text/css">

    <!-- Styles -->
    <link href="{{ asset('css/app.css') }}" rel="stylesheet">

		<style>
		  .right.item .ui.input {
		    width:auto;
		  }
		</style>

		@yield('scripts')

</head>
<body>
    <div id="app">
			<div class="ui top fluid segment" style="background-color: #2a0038;">
				<div class="ui top attached inverted secondary icon menu">
					<a href="https://www.vesselenterprise.com/" target="_blank" class="item">
						Vessel
					</a>
					<a href="{{ url('/home') }}" class="active item">
						<i class="home icon"></i>
						&nbsp;Home
					</a>
					<a class="item" href="{{ route('user.index') }}">
						<i class="user icon"></i>
						&nbsp;Users
					</a>
					<a class="item" href="{{ route('file.index') }}">
						<i class="file icon"></i>
						&nbsp;Files
					</a>
					<a class="item" href="{{ route('storage.index') }}">
						<i class="hdd icon"></i>
						&nbsp;Storage
					</a>
					<a class="item" href="{{ route('client.index') }}">
						<i class="computer icon"></i>
						&nbsp;Clients
					</a>
					<a class="item" href="{{ route('setting.index') }}">
						<i class="setting icon"></i>
						&nbsp;Configuration
					</a>
					<a class="item" href="{{ route('deployment.index') }}">
						<i class="power icon"></i>
						&nbsp;Deployment
					</a>

					<div class="right menu">

						@guest
		          <a class="right item" href="{{ route('login') }}">{{ __('Login') }}</a>
		          <a class="right item" href="{{ route('register') }}">{{ __('Register') }}</a>

						@else

							<!-- User dropdown -->
							<div class="right item">
								<div class="ui inverted labeled icon pointing dropdown link item" style="width: 150px; background-color: #fff; color: #2a0038 !important;">
									<span class="text">{{ Auth::user()->first_name }}</span>
								  <i class="dropdown icon"></i>
								  <div class="menu">
										<div class="header">Menu</div>
										<a class="item" href="{{ route('user.profile', ['id' => Auth::user()->uuid_text]) }}">
											<i class="address card icon"></i>
											My Profile
										</a>
										<a class="item" href="{{ route('logout') }}">
											<i class="file icon"></i>
											My Files
										</a>
										<a class="item" href="{{ route('logout') }}">
											<i class="question circle icon"></i>
											Help
										</a>
										<a class="item" href="{{ route('logout') }}" onclick="event.preventDefault(); document.getElementById('logout-form').submit();" style="color: #686868 !important;">
											<i class="sign out icon"></i>
											{{ __('Logout') }}
										</a>
								  </div>
								</div>
							</div>
							<form id="logout-form" action="{{ route('logout') }}" method="POST" style="display: none;">
									@csrf
							</form>

							<!-- Search -->
							<div class="right item">
								<div class="ui icon input" >
									<input type="text" placeholder="Search...">
									<i class="search link icon"></i>
								</div>
								<div class="results"></div>
							</div>
						@endguest

					</div>

				</div>
			</div>
      <main class="py-4">
        @yield('content')
      </main>
    </div>
</body>
</html>
