@extends('layouts.app')

@section ('scripts')
	<script src="{{ asset('js/tablesort.min.js') }}"></script>
	<script>
		$(document).ready( function() {
			$('table').tablesort();
			$('.ui.accordion').accordion();
		});
	</script>
@endsection

@section('content')

	<div class="ui very padded container">
		<h2 class="ui header">
			Logs
			<div class="sub header">View Client and User Log Entries</div>
		</h2>
	</div>

	<p></p>

	<div class="ui fluid stackable grid container">
		<div class="sixteen wide column">
			<div class="ui fluid styled accordion">
			  <div class="title">
			    <i class="dropdown icon"></i>
			    Search Logs
			  </div>
			  <div class="content">
			    <p class="transition hidden">
						<form id="log-search" method="post" action="/search/logs">
							@csrf
							<div class="ui fluid large icon input">
							  <input name="search_text" type="text" placeholder="Enter search words...">
							  <i class="search icon"></i>
							</div>
							<div style="padding: 4px;">
								<button class="ui olive button" onclick="$('#log-search').submit()">Search</button>
							</div>
						</form>
					</p>
			  </div>
			</div>
		</div>
	</div>

	<p></p>

	<div class="ui very padded container">
		<table class="ui selectable sortable stackable padded striped table">
			<thead>
				<tr>
					<th class="no-sort" colspan="8">
		      	{{ $logs->links() }}
		    	</th>
			  </tr>
				<tr>
					<th class="no-sort"></th>
					<th>Message</th>
					<th>Type</th>
					<th>Status</th>
					<th>Code</th>
					<th>Client</th>
					<th>User</th>
					<th>Logged At</th>
				</tr>
			</thead>
			<tbody>
				@foreach ($logs as $entry)
					<tr>
						<td>
							<div class="ui fitted checkbox">
			          <input type="checkbox"><label></label>
			        </div>
						</td>
						<td><a href=""><a href="{{ route('log.show', $entry->id) }}">{{ substr($entry->message,0,100) }}</a></td>
						<td>{{ $entry->type }}</td>
						<td>
							@if($entry->error)
								<span style="color: red">
							@else()
								<span>
							@endif()
								{{ $entry->error ? "Error" : "Info" }}
								</span>
						</td>
						<td>{{ $entry->code }}</td>
						<td><a href="{{ route('client.show', $entry->client->client_id_text) }}">{{ $entry->client->client_name }}</a></td>
						<td><a href="{{ route('user.show', $entry->user->user_id_text) }}">{{ $entry->user->first_name }} {{ $entry->user->last_name }}</a></td>
						<td>{{ $entry->logged_at }}</td>
					</tr>
				@endforeach
			</tbody>
			<tfoot>
		    <tr>
					<th colspan="8">
						<div class="ui">
							<button class="red ui button">
								<i class="icon delete"></i>
							  Delete
							</button>
						</div>
			      {{ $logs->links() }}
		    	</th>
			  </tr>
			</tfoot>
		</table>
	</div>

@endsection
