@extends('layouts.app')

@section('scripts')
	<script>
		$(document).ready( function() {

			$('.ui.form').form();
			$('.ui.accordion').accordion();

			$('.message .close')
				.on('click', function() {
					$(this)
						.closest('.message')
						.transition('fade');
				});

			$('.menu .item').tab();

			$('#upload_progress').progress();

		});
	</script>
@endsection

@section('content')

<div class="ui container">

	<h2 class="ui header">
		Manage App Client
		<div class="sub header">View and Manage Application Client details</div>
	</h2>

	<form class="ui form" action="{{ route('client.update', ['id' => $client->uuid_text]) }}" method="POST">
		@csrf
		<input type="hidden" name="_method" value="put" />

		<div class="ui padded stackable grid container segment">

			<div class="ui styled fluid accordion">

				<div class="title">
				<i class="dropdown icon"></i>
					Basic Information
				</div>
				<div class="content">
					<div class="ui stackable grid container">

						<div class="four wide column">
							<div class="field">
								<label>Name</label>
							  <input name="client_name" id="client_name" type="text" value="{{ $client->client_name }}" placeholder="">
							</div>
						</div>

						<div class="twelve wide column">
							<div class="four wide field">
								<label>Client Version</label>
								<input name="client_version" id="client_version" type="text" value="{{ $client->client_version }}" placeholder="">
							</div>
						</div>

						<div class="eight wide column">
							<div class="field">
								<label>Operating System</label>
								<input name="os" id="os" type="text" value="{{ $client->os }}" placeholder="">
							</div>
						</div>

						<div class="eight wide column">
							<div class="eight wide field">
								<label>DNS Name</label>
								<input name="dns_name" id="dns_name" type="text" value="{{ $client->dns_name }}" placeholder="">
							</div>
						</div>

						<div class="four wide column">
							<div class="field">
								<label>IP Address</label>
								<input name="ip_address" id="ip_address" type="text" value="{{ $client->ip_address }}" placeholder="">
							</div>
						</div>

						<div class="four wide column">
							<div class="field">
								<label>Domain</label>
								<input name="domain" id="domain" type="text" value="{{ $client->domain }}" placeholder="">
							</div>
						</div>

					</div>
				</div>

				<div class="title">
				<i class="dropdown icon"></i>
					Security
				</div>
				<div class="content">

					<div class="ui stackable grid container">
						<div class="eight wide column">
							<div class="field">
								<label>Access Token</label>
								<input name="token" id="token" type="text" value="{{ $client->token }}" readonly="" placeholder="">
							</div>
						</div>
					</div>
				</div>

				<div class="title">
				<i class="dropdown icon"></i>
					Stats
				</div>
				<div class="content">
					<div class="ui center aligned stackable grid container">

						<div class="sixteen wide column">
							<div class="ui segment">
								<div class="ui olive progress" data-percent="75" id="upload_progress">
								  <div class="bar"></div>
								</div>
								<div class="label">75% Backup Ratio</div>
							</div>
						</div>

						<div class="four wide column">
							<div class="field">
								<label>API Requests</label>
								100
							</div>
						</div>

						<div class="four wide column">
							<div class="field">
								<label>Total Uploads</label>
								100000
							</div>
						</div>

						<div class="four wide column">
							<div class="field">
								<label>Total Upload Filesize</label>
								1.24TB
							</div>
						</div>

						<div class="four wide column">
							<div class="field">
								<label>Associated Users</label>
									{{ count($client->users) }}
							</div>
						</div>

						<div class="four wide column">
							<div class="field">
								<label>Last Backup</label>
									{{ $client->last_backup }}
							</div>
						</div>

						<div class="four wide column">
							<div class="field">
								<label>Last Check-In</label>
									{{ $client->last_check_in }}
							</div>
						</div>

					</div>
				</div>

				<div class="title">
				<i class="dropdown icon"></i>
					Associated Users
				</div>
				<div class="content">

					<div class="ui stackable grid container">
						<div class="eight wide column">
							<div class="ui relaxed divided list">

								@foreach($client->users as $user)
								<div class="item">
							    <i class="user icon"></i>
							    <div class="content">
										<a href="{{ route('user.show', ['id' => $user->uuid_text]) }}">{{ $user->email }}</a>
									</div>
								</div>
								@endforeach

							</div>
						</div>
					</div>

				</div>

			</div>

			<div class="sixteen wide center aligned column">
				<div class="ui centered">
					<div class="ui primary submit button">{{ __('Save') }}</div>
				</div>
			</div>

		</div>

	</form>

</div>

@endsection
