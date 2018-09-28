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

			//$('#provider_type').val('{{ $provider->provider_type }}');
			$('#provider_type').dropdown('set selected', '{{ $provider->provider_type }}');
			$('.provider-active').checkbox('{{ $provider->active ? 'check' : 'uncheck' }}');

		});
	</script>
@endsection

@section('content')

<div class="ui container">

	<h2 class="ui header">
		Manage Storage Provider
		<div class="sub header">View or Manage Storage Provider for uploads</div>
	</h2>

	<form class="ui form" action="{{ route('storage.update', ['id' => $provider->uuid_text]) }}" method="POST">
		@csrf
		<input type="hidden" name="_method" value="put" />

		<div class="ui stackable grid container segment">

			<div class="sixteen wide column">
				<div class="field">
					<div class="ui toggle checkbox provider-active">
					  <input type="checkbox" name="active" id="active">
					  <label>Active</label>
					</div>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Select Provider Type</label>
					<select name="provider_type" id="provider_type" class="ui dropdown">
						<option class="item" value="vessel">Vessel</option>
						<option value="aws_s3">AWS S3</option>
						<option value="azure_blob">Azure Blob Storage</option>
						<option value="google">Google Cloud Storage</option>
						<option value="user_remote">User Remote</option>
					</select>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Provider Name</label>
				  <input name="provider_name" id="provider_name" type="text" value="{{ $provider->provider_name }}" placeholder="Enter any name">
				</div>
			</div>

			<div class="four wide column">
				<div class="field">
					<label>Bucket Name</label>
					<input name="bucket_name" id="bucket_name" type="text" value="{{ $provider->bucket_name }}" placeholder="Enter any name">
				</div>
			</div>

			<div class="four wide column">
				<div class="field">
					<label>Region</label>
					<input name="region" id="region" type="text" value="{{ $provider->region }}" placeholder="Example: us-east-1">
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Server</label>
					<div class="ui labeled input">
						<div class="ui label">
					 		https://
				 		</div>
						<input name="server" id="server" type="text" value="{{ $provider->server }}" placeholder="Enter URL">
					</div>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Access ID</label>
					<div class="ui icon input">
						<input name="access_id" id="access_id" type="text" value="{{ $provider->access_id }}" placeholder="">
						<i class="key icon"></i>
					</div>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Access Key</label>
					<div class="ui icon input">
						<input name="access_key" id="access_key" type="text" value="{{ $provider->access_key }}" placeholder="">
						<i class="key icon"></i>
					</div>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Storage Path</label>
					<input name="storage_path" id="storage_path" type="text" value="{{ $provider->storage_path }}" placeholder="Enter Bucket Name">
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Priority</label>
					<input name="priority" id="priority" type="number" value="{{ $provider->priority }}" placeholder="Enter a number from 0 to 100">
				</div>
			</div>

			<div class="sixteen wide column">
				<div class="field">
					<label>Description</label>
					<textarea name="description" id="description" rows="4">{{ $provider->description }}</textarea>
				</div>
			</div>

			<div class="center aligned sixteen wide column">
				<div class="ui primary submit button">{{ __('Save') }}</div>
			</div>

		</div>

	</form>

</div>

@endsection
