@extends('layouts.app')

@section('content')

	<h2 class="ui center aligned block header">
		 Vessel Home
		<div class="sub header">Your dashboard and visibility into what's going on</div>
	</h3>

	<div class="ui very padded centered stackable grid container">

		<!-- Recent Heartbeats -->
		<div class="four wide column">
			<div class="ui fluid stackable card">
			  <div class="content">
			    <div class="header">Recent Heartbeats</div>
			  </div>
				<div class="content">
					<div class="ui divided link list">
						@foreach ($heartbeats as $client)
						  <div class="item">
						    <i class="heart icon"></i>
						    <div class="content">
						      <a href="{{ route('client.show', $client->client_id_text) }}">{{ $client->client_name }}</a>
						      <div class="description">@ {{ $client->last_check_in }}</div>
						    </div>
						  </div>
						@endforeach
					</div>
				</div>
			</div>
		</div>

		<!-- Recent Backups -->
		<div class="four wide column">
			<div class="ui fluid stackable card">
			  <div class="content">
			    <div class="header">Recent Backups</div>
			  </div>
				<div class="content">
					<div class="ui divided link list">
						@foreach ($recentBackups as $file)
						  <div class="item">
						    <i class="heart icon"></i>
						    <div class="content">
						      <a href="{{ route('file.show', $file->file_id_text) }}">{{ $file->file_name }}</a>
						      <div class="description">@ {{ $file->last_backup }} by {{ isset($file->user) ? $file->user->user_name : "" }}</div>
						    </div>
						  </div>
						@endforeach
					</div>
				</div>
			</div>
		</div>

		<!-- Recent Errors -->
		<div class="four wide column">
			<div class="ui fluid stackable card">
			  <div class="content">
			    <div class="header">Recent Errors</div>
			  </div>
				<div class="content">
					<div class="ui divided link list">
						@foreach ($recentErrors as $error)
						  <div class="item">
						    <i class="exclamation circle icon"></i>
						    <div class="content">
						      <a href="{{ route('log.show', $error->id) }}">{{ $error->type }}</a>
						      <div class="description">@ {{ $error->logged_at }} {{ isset($error->client) ? "(" . $error->client->client_name . ")" : "" }} {{ isset($error->user) ? "(" . $error->user->user_name . ")" : "" }}</div>
						    </div>
						  </div>
						@endforeach
					</div>
				</div>
			</div>
		</div>

		<!-- Storage Providers -->
		<div class="four wide column">
			<div class="ui fluid stackable card">
			  <div class="content">
			    <div class="header">Storage Providers</div>
			  </div>
				<div class="content">
					<div class="ui divided link list">
						@foreach ($storageProviders as $provider)
						  <div class="item">
						    <i class="hdd icon"></i>
						    <div class="content">
						      <a href="{{ route('storage.show', $provider->provider_id_text) }}">{{ $provider->provider_name }}</a>
						      <div class="description"></div>
						    </div>
						  </div>
						@endforeach
					</div>
				</div>
			</div>
		</div>

	</div>


@endsection
