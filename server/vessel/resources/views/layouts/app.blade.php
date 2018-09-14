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
				$('.vessel-menu-item').dropdown({
					on : 'hover'
				});
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
				<div id="vessel-header-menu" class="ui top attached inverted secondary icon menu">

					<a href="https://www.vesselenterprise.com/" target="_blank" class="item">
						Vessel
					</a>
					<a href="{{ url('/home') }}" class="active item">
						<i class="home icon"></i>
						&nbsp;Home
					</a>

					<div class="ui dropdown icon item vessel-menu-item">
						<i class="user icon"></i>
						<span class="text">
							&nbsp;<a class="header-link" href="{{ route('user.index') }}">Users</a>
						</span>
						<div class="menu">
							<a class="item" href="{{ route('user.index') }}">Manage Users</a>
							<a class="item" href="{{ route('user_setting.index') }}">Manage User Settings</a>
							<a class="item" href="{{ route('user_role.index') }}">Manage Roles</a>
							<a class="item" href="">LDAP Import</a>

						</div>
					</div>

					<div class="ui dropdown icon item vessel-menu-item">
						<i class="file icon"></i>
						<span class="text">
							&nbsp;<a class="header-link" href="{{ route('file.index') }}">Files</a>
						</span>
						<div class="menu">
							<a class="item" href="{{ route('file.index') }}">Manage Files</a>
							<a class="item" href="{{ route('file_ignore_type.index') }}">Ignored Filetypes</a>
							<a class="item" href="{{ route('file_ignore_dir.index') }}">Ignored Directories</a>
							<a class="item" href="">Stats</a>
						</div>
					</div>

					<div class="ui dropdown icon item vessel-menu-item">
						<i class="hdd icon"></i>
						<span class="text">
							&nbsp;<a class="header-link" href="{{ route('storage.index') }}">Storage</a>
						</span>
						<div class="menu">
							<a class="item" href="{{ route('storage.index') }}">Manage Storage Providers</a>
							<a class="item" href="">Storage Utilization</a>
						</div>
					</div>

					<div class="ui dropdown icon item vessel-menu-item">
						<i class="computer icon"></i>
						<span class="text">
							&nbsp;<a class="header-link" href="{{ route('client.index') }}">Clients</a>
						</span>
						<div class="menu">
							<a class="item" href="{{ route('client.index') }}">Manage Clients</a>
							<a class="item" href="">Download Client</a>
							<a class="item" href="">LDAP Import</a>
						</div>
					</div>

					<div class="ui dropdown icon item vessel-menu-item">
						<i class="setting icon"></i>
						<span class="text">
							&nbsp;<a class="header-link" href="{{ route('setting.index') }}">Configuration</a>
						</span>
						<div class="menu">
							<a class="item" href="{{ route('setting.index') }}">Manage Settings</a>
							<a class="item" href="">Email and Alerts</a>
							<a class="item" href="">Network and Bandwidth</a>
							<a class="item" href="">Encryption</a>
						</div>
					</div>

					<div class="ui dropdown icon item vessel-menu-item">
						<i class="power icon"></i>
						<span class="text">
							&nbsp;<a class="header-link" href="{{ route('deployment.index') }}">Deployment</a>
						</span>
						<div class="menu">
							<a class="item" href="{{ route('deployment.index') }}">Manage Deployments</a>
							<a class="item" href="">Client Deployment</a>
						</div>
					</div>

					<div class="ui dropdown icon item vessel-menu-item">
						<i class="help icon"></i>
						<span class="text">
							&nbsp;Help
						</span>
						<div class="menu">
							<a class="item" href="">Documentation</a>
							<a class="item" target="_blank" href="https://github.com/VesselEnterprise/vessel-backup">GitHub</a>
							<a class="item" href="">About</a>

						</div>
					</div>

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
								<div class="ui big icon input" >
									<input type="text" id="search-text" name="search-text" placeholder="Search..." >
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
