@extends('layouts.app')

@section ('scripts')
	<script src="{{ asset('js/tablesort.min.js') }}"></script>
	<script>
		$(document).ready( function() {
			$('table').tablesort();
			$('.ui.accordion').accordion();
			$('.message .close')
				.on('click', function() {
					$(this)
						.closest('.message')
						.transition('fade');
				});
		});
	</script>
@endsection

@section('content')

	@if( isset($success) )
		<div class="ui very padded container">
			<div class="ui positive message">
				<i class="close icon"></i>
				<div class="header">
					{{ $success }}
				</div>
			</div>
		</div>
		<p></p>
	@endif

	<div class="ui very padded container">
		<h2 class="ui header">
			LDAP Servers
			<div class="sub header">View and Manage LDAP Servers</div>
		</h2>
	</div>

	<p></p>

	<div class="ui very padded container">
		<table class="ui selectable sortable stackable padded striped table">
			<thead>
				<tr>
					<th class="no-sort" colspan="8">
		      	{{ $ldapServers->links() }}
		    	</th>
			  </tr>
				<tr>
					<th class="no-sort"></th>
					<th>Name</th>
					<th>Server</th>
					<th>Status</th>
				</tr>
			</thead>
			<tbody>
				@foreach ($ldapServers as $server)
					<tr>
						<td>
							<div class="ui fitted checkbox">
			          <input type="checkbox"><label></label>
			        </div>
						</td>
						<td><a href="{{ route('ldap.show', $server->id) }}">{{ $server->name }}</a></td>
						<td>{{ $server->server }}</td>
						<td>{{ $server->status }}</td>
					</tr>
				@endforeach
			</tbody>
			<tfoot>
		    <tr>
					<th colspan="8">
						<div class="ui">
							<button id="create_server" class="ui primary button">
								<i class="icon plus"></i>
							  Create New
							</button>
							<button class="red ui button">
								<i class="icon delete"></i>
							  Delete
							</button>
						</div>
			      {{ $ldapServers->links() }}
		    	</th>
			  </tr>
			</tfoot>
		</table>
	</div>

@endsection
