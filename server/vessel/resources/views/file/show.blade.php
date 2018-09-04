@extends('layouts.app')

@section('scripts')
	<script>
		$(document).ready( function() {

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
		View File
		<div class="sub header">View file details and download</div>
	</h2>

	@if( isset($status) )
		<div class="ui positive message">
			<i class="close icon"></i>
			<div class="header">
				File has been updated
			</div>
		</div>
	@endif

	<form method="POST" action="{{ route('file.update', ['id' => $file->file_id_text]) }}" class="ui form segment">
		@csrf

	<div class="ui stackable grid container">
	  <div class="eight wide column">
			<div class="ui segment">
				<p><b>File Name</b></p>
				<span>{{ $file->file_name }}</span>
			</div>
		</div>
	  <div class="four wide column">
			<div class="ui segment">
				<p><b>User</b></p>
				<span><a href="{{ route('user.show', $file->user->user_id_text) }}">{{ $file->user->first_name . " " . $file->user->last_name }}</a></td></span>
			</div>
		</div>
		<div class="four wide column">
			@if($file->uploaded == 1)
			<div class="ui segment green">
			@else
			<div class="ui segment red">
			@endif
				<p><b>Status</b></p>
				<span>{{ $file->uploaded ? 'Uploaded' : 'Pending Upload' }}</span>
			</div>
		</div>
	  <div class="eight wide column">
			<div class="ui segment">
				<p><b>File Path</b></p>
				<div class="ui list">
				  <div class="item">
				    <i class="folder icon"></i>
				    <div class="content">
				      <div class="description">{{ $file->filePath->file_path }}</div>
				      <div class="list">
								<div class="item">
				          <i class="file icon"></i>
				          <div class="content">
				            <div class="description">{{ $file->file_name }}</div>
				          </div>
								</div>
			        </div>
						</div>
					</div>
				</div>
			</div>
		</div>
	  <div class="four wide column">
			<div class="ui segment">
				<p><b>File Type</b></p>
				<span>{{ $file->file_type }}</span>
			</div>
		</div>
		<div class="four wide column">
			<div class="ui segment">
				<p><b>File Size</b></p>
				<span>{{ $file->file_size }}</span>
			</div>
		</div>
		<div class="eight wide column">
			<div class="ui segment">
				<p><b>Hash</b></p>
				<span>{{ $file->hash }}</span>
			</div>
		</div>
		<div class="four wide column">
			<div class="ui segment">
				<p><b>Storage Provider</b></p>
				<span><a href="{{ route('storage.show', $file->storageProvider->provider_id_text ) }}">{{ $file->storageProvider->provider_name }}</a></span>
			</div>
		</div>
		<div class="four wide column">
			<div class="ui segment">
				<p><b>Last Backup</b></p>
				<span>{{ $file->last_backup == "" ? 'Never' : $file->last_back }}</span>
			</div>
		</div>
	</div>

	<div class="ui left aligned fluid stackable grid container">
		<div class="sixteen wide column">
			<button class="ui large primary button">
				<i class="download icon"></i>
				Download
			</button>
			<button class="ui large button">
				<i class="share icon"></i>
				Share
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
