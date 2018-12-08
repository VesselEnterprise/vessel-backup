@extends('layouts.app')

@section('content')

<div class="ui very padded container">

	<h2 class="ui header">
		View Log Entry
		<div class="sub header"></div>
	</h2>

	<div class="ui stackable grid container">

		<div class="four wide column">
			<div class="ui card">
				<div class="content">
					<div class="header">Status</div>
					<div class="meta">Error or Info</div>
					<div class="description">
						<p>
							@if( $entry->error )
								<div class="ui red label">
							@else
								<div class="ui label">
							@endif
								{{ $entry->error ? 'Error' : 'Info' }}
							</div>
						</p>
					</div>
				</div>
			</div>
		</div>

		<div class="four wide column field">
			<div class="ui card">
			  <div class="content">
			    <div class="header">Type</div>
			    <div class="meta">Entry type</div>
			    <div class="description">
			      <p>
							<div class="ui blue label">{{ $entry->type }}</div>
						</p>
			    </div>
			  </div>
			</div>
		</div>

		<div class="four wide column">
			<div class="ui card">
			  <div class="content">
			    <div class="header">Exception</div>
			    <div class="meta">Exception name</div>
			    <div class="description">
			      <p>
							<div class="ui orange label">{{ $entry->exception }}</div>
						</p>
			    </div>
			  </div>
			</div>
		</div>

		<div class="four wide column">
			<div class="ui card">
			  <div class="content">
			    <div class="header">Code</div>
			    <div class="meta">Response, Error, or Status Code</div>
			    <div class="description">
			      <p>
							<div class="ui label">{{ $entry->code }}</div>
						</p>
			    </div>
			  </div>
			</div>
		</div>

		<div class="four wide column">
			<div class="ui card">
				<div class="content">
					<div class="header">Client</div>
					<div class="meta">Client machine for log entry</div>
					<div class="description">
						<p>
							<div class="ui label">
								<a href="{{ route('client.show', $entry->client->client_id_text) }}">{{ $entry->client->client_name }}</a>
							</div>
						</p>
					</div>
				</div>
			</div>
		</div>

		<div class="four wide column">
			<div class="ui card">
				<div class="content">
					<div class="header">User</div>
					<div class="meta">User associated with log entry</div>
					<div class="description">
						<p>
							<div class="ui label">
								<a href="{{ route('user.show', $entry->user->user_id_text) }}">{{ $entry->user->first_name }} {{ $entry->user->last_name }}</a>
							</div>
						</p>
					</div>
				</div>
			</div>
		</div>

		<div class="sixteen wide column">
			<div class="ui card fluid">
				<div class="content">
					<div class="header">Message</div>
					<div class="meta">Log message</div>
					<div class="description">
						<p>
							<div class="ui form">
							  <div class="field">
							    <label></label>
							    <textarea rows="8">{{ $entry->message }}</textarea>
							  </div>
							</div>
						</p>
					</div>
				</div>
			</div>
		</div>

		<div class="sixteen wide column">
			<div class="ui card fluid">
				<div class="content">
					<div class="header">Payload</div>
					<div class="meta">Data content or payload</div>
					<div class="description">
						<p>
							<div class="ui form">
								<div class="field">
									<label></label>
									<textarea rows="8">{{ $entry->payload }}</textarea>
								</div>
							</div>
						</p>
					</div>
				</div>
			</div>
		</div>

	</div>

</div>
@endsection
