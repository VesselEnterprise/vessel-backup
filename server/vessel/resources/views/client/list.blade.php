@extends('layouts.app')

@section ('scripts')
	<script src="{{ asset('js/tablesort.min.js') }}"></script>
	<script>
		$(document).ready( function() {

			$('.message .close')
				.on('click', function() {
					$(this)
						.closest('.message')
						.transition('fade');
				});

			$('.menu .item').tab();

			var list_form = $('#list_form');

			$('table').tablesort();

			$('#delete_selected').click(function() {
				list_form.attr('action', '{{ route('client.destroyMultiple') }}');
				list_form.submit();
			});

			$('#create_client').click(function(e) {
				e.preventDefault();
				window.location.href= '{{ url('client/create') }}';
			});

		});
	</script>
@endsection

@section('content')

	<div class="ui very padded container">

		@if(isset($success))
		<div class="ui positive message">
			<i class="close icon"></i>
			<div class="header">
				{{ $success }}
			</div>
		</div>
		@endif

		@if(isset($error))
		<div class="ui negative message">
			<i class="close icon"></i>
			<div class="header">
				{{ $error }}
			</div>
		</div>
		@endif

		<h2 class="ui header">
			App Clients
			<div class="sub header">View and Manage Application Clients</div>
		</h2>
	</div>

	<form id="list_form" action="" method="POST">
		@csrf

		<div class="ui very padded container">
			<table class="ui selectable sortable stackable padded striped table">
				<thead>
					<tr>
						<th class="no-sort" colspan="9">
			      	{{ $clients->links() }}
			    	</th>
				  </tr>
					<tr>
						<th class="no-sort"></th>
						<th>Name</th>
						<th>Operating System</th>
						<th>DNS Name</th>
						<th>IP Address</th>
						<th>Domain</th>
						<th>Client Version</th>
						<th>Last Check In</th>
						<th>Last Backup</th>
					</tr>
				</thead>
				<tbody>
					@foreach ($clients as $client)
						<tr>
							<td>
								<div class="ui fitted checkbox">
				          <input name="selectedIds[]" type="checkbox" value="{{ $client->uuid_text }}"><label></label>
				        </div>
							</td>
							<td><a href="{{ route('client.show', $client->uuid_text) }}">{{ $client->client_name }}</td>
							<td>{{ $client->os }}</td>
							<td>{{ $client->dns_name }}</td>
							<td>{{ $client->ip_address }}</td>
							<td>{{ $client->domain }}</td>
							<td>{{ $client->client_version }}</td>
							<td>{{ $client->last_check_in }}</td>
							<td>{{ $client->last_backup }}</td>
						</tr>
					@endforeach
				</tbody>
				<tfoot>
			    <tr>
						<th colspan="9">
							<div class="ui">
								<button id="create_client" class="ui primary button">
									<i class="icon plus"></i>
								  Create New
								</button>
								<button id="delete_selected" class="red ui button">
									<i class="icon delete"></i>
								  Delete
								</button>
							</div>
				      {{ $clients->links() }}
			    	</th>
				  </tr>
				</tfoot>
			</table>
		</div>

	</form>

@endsection
