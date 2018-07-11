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

		@yield('scripts')

</head>
<body>
    <div id="app">
			<div class="ui inverted segment" style="background-color: #2a0038;">
				<div class="ui stackable inverted secondary menu">
					<a href="{{ url('/home') }}" class="item">
						Vessel
					</a>
					<a href="{{ url('/home') }}" class="active item">
						Home
					</a>
					<a class="item" href="{{ route('user.index') }}">
						Users
					</a>
					<a class="item">
						Files
					</a>
					<a class="item">
						Storage
					</a>
					<a class="item">
						Clients
					</a>
					<a class="item">
						Configuration
					</a>
					<a class="item">
						Deployment
					</a>
					<div class="right menu">
						@guest
		          <a class="item" href="{{ route('login') }}">{{ __('Login') }}</a>
		          <a class="item" href="{{ route('register') }}">{{ __('Register') }}</a>

						@else

							<!-- User dropdown -->
							<div class="ui container" style="width: 220px;">
								<div class="ui very padded fluid menu" style="background-color: #fbffdd; width: 220px;">
									<div class="ui fluid pointing dropdown link item" style="color: #2a0038 !important;">
										<span class="right text">{{ Auth::user()->first_name }}</span>
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
											<a class="item" href="{{ route('logout') }}" onclick="event.preventDefault(); document.getElementById('logout-form').submit();" style="color: #aaa !important;">
												<i class="sign out icon"></i>
												{{ __('Logout') }}
											</a>
									  </div>
									</div>
								</div>
								<form id="logout-form" action="{{ route('logout') }}" method="POST" style="display: none;">
										@csrf
								</form>
							</div>

							<!-- Search -->
							<div class="ui action input">
								<input type="text" placeholder="Search...">
								<button class="ui icon button">
									<i class="search icon"></i>
								</button>
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
