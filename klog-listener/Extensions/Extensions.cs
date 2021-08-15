using klog_listener.Contexts;
using Microsoft.AspNetCore.Builder;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using System.IO;
using System.Reflection;

namespace klog_listener.Extensions
{
    public static class Extensions
    {
        public static void AddKeyLogStore(this IServiceCollection services, IConfiguration configuration)
        {
            var connection = configuration.GetConnectionString("KeyLogConnection");

            if (connection.Contains("~"))
            {
                var myDirectory = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                connection = connection.Replace("~", myDirectory);
            }

            services.AddDbContext<KeyLogContext>(
                options => options.UseSqlite(connection),
                ServiceLifetime.Scoped,
                ServiceLifetime.Singleton);
        }

        public static void UseKeyLogStore(this IApplicationBuilder app)
        {
            var scopeFactory = app.ApplicationServices.GetRequiredService<IServiceScopeFactory>();
            using var serviceScope = scopeFactory.CreateScope();
            var context = serviceScope.ServiceProvider.GetService<KeyLogContext>();
            context?.Database.EnsureCreated();
        }
    }
}
