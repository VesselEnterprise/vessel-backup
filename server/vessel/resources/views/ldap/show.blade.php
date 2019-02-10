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

			$('#type').dropdown('set selected', '{{ $ldapServer->type }}');

		});

		function runImport() {
			$.get('{{ route('ldap.import', $ldapServer->id) }}', function(data) {
				alert(data);
			});
		}

	</script>
@endsection

@section('content')

<div class="ui container">

	<h2 class="ui header">
		Manage LDAP Server
		<div class="sub header"></div>
	</h2>

	@if( isset($success) )
		<div class="ui positive message">
			<i class="close icon"></i>
			<div class="header">
				{{ $success }}
			</div>
		</div>
	@endif

	<form method="POST" action="{{ route('ldap.update', ['id' => $ldapServer->id]) }}" class="ui form segment">
		@csrf
		@method('PUT')

		<div class="ui fluid stackable grid container">

			<div class="eight wide column">
				<div class="field">
					<label>Server Type</label>
					<select name="type" id="type" class="ui dropdown" required>
						<option value="active_directory">Active Directory</option>
						<option value="open_ldap">OpenLDAP</option>
						<option value="free_ipa">FreeIPA</option>
					</select>
				</div>
			</div>

		  <div class="eight wide column">
				<div class="field">
					<label>Name</label>
					<input type="text" id="name" name="name" placeholder="Display Name" value="{{ $ldapServer->name }}" required>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Server</label>
					<input type="text" id="server" name="server" placeholder="Server" value="{{ $ldapServer->server }}" required>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Port</label>
					<input type="number" id="port" name="port" placeholder="Port" value="{{ $ldapServer->port }}" required>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Username</label>
					<input type="text" id="username" name="username" placeholder="Username" value="{{ $ldapServer->username }}" required>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Password</label>
					<input type="password" id="password" name="password" placeholder="Password" value="{{ $ldapServer->password }}" required>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Base DN</label>
					<input type="text" id="dn" name="dn" placeholder="Base DN" value="{{ $ldapServer->dn }}" required>
				</div>
			</div>

		</div>

		<div class="ui left aligned fluid stackable grid container">
			<div class="sixteen wide column">

				<div class="ui large primary submit button">{{ __('Save') }}</div>

				<button type="button" class="ui large secondary button" onclick="runImport()">
					<i class="sync icon"></i>
					Run Import
				</button>

				<button class="ui large button">
					<i class="delete icon"></i>
					Delete
				</button>

			</div>
		</div>

	</form>
</div>
@endsection
