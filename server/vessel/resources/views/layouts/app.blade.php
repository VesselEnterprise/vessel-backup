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

		<!-- Vessel -->
		<link rel="stylesheet" type="text/css" href="{{ asset('css/vessel.css') }}">

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
				$('.ui.sticky')
				  .sticky({
				    context: '#context',
						jitter: 100
				  });
				@if(Agent::isMobile())
					$('.ui.sidebar')
						.sidebar({context: $('#side-nav'), duration: 200})
						.sidebar('attach events', '.menu .item');
				@endif
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

			@if(!Agent::isMobile())

				<div class="ui top fluid segment sticky" style="background-color: #2a0038;">

						<div id="vessel-header-menu" class="ui top inverted secondary icon menu">

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
									<a class="item" href="{{ route('ldap.index') }}">LDAP</a>
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
								<i class="heartbeat icon"></i>
								<span class="text">
									&nbsp;<a class="header-link" href="{{ route('deployment.index') }}">Monitoring</a>
								</span>
								<div class="menu">
									<a class="item" href="{{ route('log.index') }}">Logs</a>
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
										<div class="ui right aligned category search" style="width: 300px">
											<div class="ui fluid large icon input">
												<input class="prompt" type="text" id="search-text" name="search-text" placeholder="Search...">
												<i class="search icon"></i>
											</div>
											<div class="results"></div>
										</div>
									</div>
								@endguest

							</div>

						</div>

					</div>

				@else

					<div class="ui top attached menu" style="background-color: #2a0038;">
						<a class="item" style="color: #fff;">
							<i class="sidebar icon"></i>
							Menu
						</a>
					</div>
					<div id="side-nav" class="ui bottom attached segment pushable">
						<div class="ui left vertical sidebar menu" style="">

							<a href="https://www.vesselenterprise.com/" target="_blank" class="item">
								Vessel
							</a>

							<a href="{{ url('/home') }}" class="item">
								<i class="home icon"></i>
								Home
							</a>

							<a class="item" href="{{ route('user.index') }}">
								<i class="user icon"></i>
								Users
							</a>

							<a class="item" href="{{ route('file.index') }}">
								<i class="file icon"></i>
								Files
							</a>

							<a class="item" href="{{ route('storage.index') }}">
								<i class="hdd icon"></i>
								Storage
							</a>

							<a class="item" href="{{ route('client.index') }}">
								<i class="computer icon"></i>
								Clients
							</a>

							<a class="item" href="{{ route('setting.index') }}">
								<i class="setting icon"></i>
								Configuration
							</a>

							<a class="item" href="{{ route('deployment.index') }}">
								<i class="power icon"></i>
								Deployment
							</a>

							<a class="item" href="{{ route('deployment.index') }}">
								<i class="heartbeat icon"></i>
								Monitoring
							</a>

							<a class="item" href="{{ route('deployment.index') }}">
								<i class="help icon"></i>
								Help
							</a>

						</div>
						<div class="pusher">
								@yield('content')
						</div>
					</div>

				@endif

				@if(!Agent::isMobile())
					<div id="context">
			        @yield('content')
					</div>
				@endif

    </div>

</body>
</html>
