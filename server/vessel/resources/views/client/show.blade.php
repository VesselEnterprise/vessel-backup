@extends('layouts.app')

@section('scripts')
	<script>
		$(document).ready( function() {

			$('.ui.form').form();

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

		<div class="ui stackable grid container segment">

			<div class="ui stackable top attached tabular menu">
				<a class="item active" data-tab="first">General</a>
				<a class="item" data-tab="second">Security</a>
				<a class="item" data-tab="third">Stats</a>
				<a class="item" data-tab="fourth">Associated Users</a>
			</div>

			<div class="ui padded bottom attached tab segment active" data-tab="first">
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

			<div class="ui padded bottom attached tab segment" data-tab="second">
				<div class="ui stackable grid container">
					<div class="eight wide column">
						<div class="field">
							<label>Access Token</label>
							<input name="token" id="token" type="text" value="{{ $client->token }}" readonly="" placeholder="">
						</div>
					</div>
				</div>
			</div>

			<div class="ui very padded bottom attached tab segment" data-tab="third">
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

			<div class="ui padded bottom attached tab segment" data-tab="fourth">
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

			<div class="center aligned sixteen wide column">
				<div class="ui primary submit button">{{ __('Save') }}</div>
			</div>

		</div>

	</form>

</div>

@endsection
