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

		});
	</script>
@endsection

@section('content')

<div class="ui container">

	<h2 class="ui header">
		Create Storage Provider
		<div class="sub header">Configure a new storage provider for user file uploads</div>
	</h2>

	<form class="ui form" action="{{ route('storage.store') }}" method="POST">
		@csrf

		<div class="ui stackable grid container segment">

			<div class="eight wide column">
				<div class="field">
					<label>Select Provider Type</label>
					<select name="provider_type" id="provider_type" class="ui dropdown">
						<option class="item" value="vessel">Vessel</option>
						<option value="aws_s3">AWS S3</option>
						<option value="azure">Azure Storage</option>
						<option value="google">Google Cloud Storage</option>
						<option value="user_remote">User Remote</option>
					</select>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Provider Name</label>
				  <input name="provider_name" id="provider_name" type="text" placeholder="Enter any name">
				</div>
			</div>

			<div class="four wide column">
				<div class="field">
					<label>Bucket Name</label>
					<input name="bucket_name" id="bucket_name" type="text" placeholder="Enter any name">
				</div>
			</div>

			<div class="four wide column">
				<div class="field">
					<label>Region</label>
					<input name="region" id="region" type="text" placeholder="Example: us-east-1">
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Server</label>
					<div class="ui labeled input">
						<div class="ui label">
					 		https://
				 		</div>
						<input name="server" id="server" type="text" placeholder="Enter URL">
					</div>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Access ID</label>
					<div class="ui icon input">
						<input name="access_id" id="access_id" type="text" placeholder="">
						<i class="key icon"></i>
					</div>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Access Key</label>
					<div class="ui icon input">
						<input name="access_key" id="access_key" type="text" placeholder="">
						<i class="key icon"></i>
					</div>
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Storage Path</label>
					<input name="storage_path" id="storage_path" type="text" placeholder="Enter Bucket Name">
				</div>
			</div>

			<div class="eight wide column">
				<div class="field">
					<label>Priority</label>
					<input name="priority" id="priority" type="number" placeholder="Enter a number from 0 to 100">
				</div>
			</div>

			<div class="sixteen wide column">
				<div class="field">
					<label>Description</label>
					<textarea name="description" id="description" rows="4"></textarea>
				</div>
			</div>

			<div class="center aligned sixteen wide column">
				<div class="ui primary submit button">{{ __('Submit') }}</div>
			</div>

		</div>

	</form>

</div>

@endsection
